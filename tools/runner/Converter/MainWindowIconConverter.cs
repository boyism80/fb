using System;
using System.Globalization;
using System.Windows;

namespace runner.Converter
{
    public class MainWindowIconConverter : BaseValueConverter<MainWindowIconConverter>
    {
        public override object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var windowState = (WindowState)value;

            if (windowState == System.Windows.WindowState.Maximized)
                return "/Image/normalize.png";
            else
                return "/Image/maximize.png";
        }

        public override object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
