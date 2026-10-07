using System.Globalization;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using MapEditor.Asset;
using MapEditor.Table;

namespace MapEditor.Converter
{
    /// <summary>
    /// Enum or string property ↔ RadioButton/MenuItem IsChecked; ConverterParameter is the member name or the string.
    /// </summary>
    public class EnumBoolConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return string.Equals(value?.ToString(), parameter?.ToString(), StringComparison.OrdinalIgnoreCase);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is not true)
                return Binding.DoNothing;

            return targetType == typeof(string) ? parameter.ToString() : Enum.Parse(targetType, parameter.ToString());
        }
    }

    /// <summary>
    /// [ClientAssets, DoorTable, DoorModel, revision] → the door's close (or open with ConverterParameter "open")
    /// objects drawn side by side, so the joined shape is visible.
    /// </summary>
    public class DoorStripConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            if (values.Length < 3 || values[0] is not ClientAssets assets || values[1] is not DoorTable table || values[2] is not DoorModel model)
                return null;

            var open = parameter?.ToString() == "open";
            var objects = model.Pairs.Select(id => table.FindPair(id)).Select(pair => pair == null ? 0 : open ? pair.Open : pair.Close).ToList();
            if (objects.Count == 0)
                return null;

            var cell = assets.CellPixels;
            var rows = Math.Max(1, objects.Max(id => assets.Objects.Find(id)?.Frames.Length ?? 1));
            var width = objects.Count * cell;
            var height = rows * cell;
            var pixels = new uint[width * height];
            for (int i = 0; i < objects.Count; i++)
                assets.DrawObject(pixels, width, height, i * cell, (rows - 1) * cell, objects[i]);

            var bitmap = BitmapSource.Create(width, height, assets.Dpi, assets.Dpi, PixelFormats.Pbgra32, null, pixels, width * 4);
            bitmap.Freeze();
            return bitmap;
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }

    /// <summary>
    /// [ClientAssets, object id] → object stack thumbnail.
    /// </summary>
    public class ObjectThumbnailConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            if (values.Length < 2 || values[0] is not ClientAssets assets || values[1] is not int id)
                return null;

            return Thumbnail.Object(assets, id);
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }

    /// <summary>
    /// [ClientAssets, npc/mob NameEntry, direction?] → idle sprite. Without a direction value the sprite faces BOTTOM.
    /// </summary>
    public class MonsterThumbnailConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            if (values.Length < 2 || values[0] is not ClientAssets assets || values[1] is not NameEntry info)
                return null;

            var direction = values.Length > 2 && values[2] is int value ? value : 2;
            return Thumbnail.Monster(assets, info.Look, info.Color, direction);
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }

    /// <summary>
    /// [DoorModel, the open map's List&lt;MapDoor&gt;] → positions where the definition is used, e.g. "(3, 5) (40, 7)".
    /// </summary>
    public class DoorUsageConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            if (values.Length < 2 || values[0] is not DoorModel model || values[1] is not List<MapDoor> doors)
                return "";

            return string.Join(" ", doors.Where(d => d.Model == model).Select(d => $"({d.X}, {d.Y})"));
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }

    public class NullToBoolConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value != null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }

    public class BoolToVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value is true ? Visibility.Visible : Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
