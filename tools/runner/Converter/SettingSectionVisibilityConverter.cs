using Runner.ViewModel;
using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace Runner.Converter
{
    public class SettingSectionVisibilityConverter : BaseValueConverter<SettingSectionVisibilityConverter>
    {
        public override object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is not SettingSection current || parameter is not string sectionName)
                return Visibility.Collapsed;

            if (!Enum.TryParse<SettingSection>(sectionName, out var target))
                return Visibility.Collapsed;

            return current == target ? Visibility.Visible : Visibility.Collapsed;
        }

        public override object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
