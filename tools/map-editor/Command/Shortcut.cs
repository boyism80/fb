using System.ComponentModel;
using System.Windows.Input;

namespace MapEditor.Command
{
    /// <summary>
    /// One key or mouse side-button press with modifiers. Text form: "Ctrl+Shift+S", "Alt+Left", "Delete", "1",
    /// "XButton1". Key names are System.Windows.Input.Key names, except D0-D9 which are written as 0-9.
    /// </summary>
    public readonly record struct Gesture(ModifierKeys Modifiers, Key Key, MouseButton? Button)
    {
        /// <summary>
        /// Mouse side buttons are the only mouse gestures; the other buttons belong to the map canvas.
        /// </summary>
        public static bool IsGestureButton(MouseButton button) => button == MouseButton.XButton1 || button == MouseButton.XButton2;

        public static bool IsModifierKey(Key key)
        {
            return key is Key.LeftCtrl or Key.RightCtrl or Key.LeftShift or Key.RightShift or Key.LeftAlt or Key.RightAlt or Key.LWin or Key.RWin or Key.System;
        }

        /// <summary>
        /// A gesture without Ctrl/Alt would type into a focused text box, so it is skipped there.
        /// </summary>
        public bool TypesText => Button == null && (Modifiers & (ModifierKeys.Control | ModifierKeys.Alt)) == 0;

        public static bool TryParse(string text, out Gesture gesture)
        {
            gesture = default;
            var parts = text.Split('+', StringSplitOptions.TrimEntries | StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length == 0)
                return false;

            var modifiers = ModifierKeys.None;
            foreach (var part in parts[..^1])
            {
                if (part.Equals("Ctrl", StringComparison.OrdinalIgnoreCase))
                    modifiers |= ModifierKeys.Control;
                else if (part.Equals("Shift", StringComparison.OrdinalIgnoreCase))
                    modifiers |= ModifierKeys.Shift;
                else if (part.Equals("Alt", StringComparison.OrdinalIgnoreCase))
                    modifiers |= ModifierKeys.Alt;
                else
                    return false;
            }

            // Enum.TryParse also accepts numbers ("3" is MouseButton.XButton1), so only names reach it.
            var last = parts[^1];
            if (last.Length == 1 && char.IsDigit(last[0]))
            {
                gesture = new Gesture(modifiers, Key.D0 + (last[0] - '0'), null);
                return true;
            }
            else if (char.IsLetter(last[0]) == false)
            {
                return false;
            }
            else if (Enum.TryParse<MouseButton>(last, true, out var button) && IsGestureButton(button))
            {
                gesture = new Gesture(modifiers, Key.None, button);
                return true;
            }
            else if (Enum.TryParse<Key>(last, true, out var key) && key != Key.None && IsModifierKey(key) == false)
            {
                gesture = new Gesture(modifiers, key, null);
                return true;
            }
            else
            {
                return false;
            }
        }

        public override string ToString()
        {
            var text = "";
            if (Modifiers.HasFlag(ModifierKeys.Control))
                text += "Ctrl+";
            if (Modifiers.HasFlag(ModifierKeys.Shift))
                text += "Shift+";
            if (Modifiers.HasFlag(ModifierKeys.Alt))
                text += "Alt+";

            if (Button is MouseButton button)
                return text + button;
            else if (Key >= Key.D0 && Key <= Key.D9)
                return text + (char)('0' + (Key - Key.D0));
            else
                return text + Key;
        }
    }

    /// <summary>
    /// An editor action that can be bound to gestures. Text holds the gestures separated by ", ".
    /// </summary>
    public class ShortcutAction : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public string Id { get; init; }
        public string Category { get; init; }
        public string Label { get; init; }
        public string Default { get; init; } = "";
        public Action Execute { get; init; }
        public string Text { get; set; } = "";
        public List<Gesture> Gestures { get; private set; } = new List<Gesture>();

        private void OnTextChanged()
        {
            Gestures = Split(Text);
        }

        public static List<Gesture> Split(string text)
        {
            var gestures = new List<Gesture>();
            foreach (var part in (text ?? "").Split(',', StringSplitOptions.TrimEntries | StringSplitOptions.RemoveEmptyEntries))
            {
                if (Gesture.TryParse(part, out var gesture))
                    gestures.Add(gesture);
            }
            return gestures;
        }
    }
}
