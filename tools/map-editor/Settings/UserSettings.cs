using System.IO;
using System.Text;
using System.Text.Json;

namespace MapEditor.Settings
{
    /// <summary>
    /// Per-user preferences in %APPDATA%\fb-map-editor\user.json, kept apart from appsettings.json so builds and
    /// repository updates do not overwrite them. New preference groups go here as more properties.
    /// </summary>
    public class UserSettings
    {
        private static readonly JsonSerializerOptions JsonOptions = new JsonSerializerOptions
        {
            WriteIndented = true,
            Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping,
        };

        /// <summary>
        /// Shortcut id → gestures separated by ", " (e.g. "Alt+Left, XButton1"). Missing ids use the defaults.
        /// </summary>
        public Dictionary<string, string> Shortcuts { get; set; } = new Dictionary<string, string>();

        public static string FilePath => Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "fb-map-editor", "user.json");

        /// <summary>
        /// A missing file gives defaults. A broken file also gives defaults and the reason in error, and is not
        /// overwritten until the user saves.
        /// </summary>
        public static UserSettings Load(out string error)
        {
            error = null;
            if (File.Exists(FilePath) == false)
                return new UserSettings();

            try
            {
                var settings = JsonSerializer.Deserialize<UserSettings>(File.ReadAllText(FilePath, Encoding.UTF8), JsonOptions) ?? new UserSettings();
                settings.Shortcuts ??= new Dictionary<string, string>();
                return settings;
            }
            catch (Exception e)
            {
                error = $"{FilePath} 읽기 실패, 기본값 사용: {e.Message}";
                return new UserSettings();
            }
        }

        public void Save()
        {
            Directory.CreateDirectory(Path.GetDirectoryName(FilePath));
            File.WriteAllText(FilePath, JsonSerializer.Serialize(this, JsonOptions), new UTF8Encoding(false));
        }
    }
}
