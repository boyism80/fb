using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;

namespace runner.Control
{
    /// <summary>
    /// Interaction logic for SystemButton.xaml
    /// </summary>
    public partial class SystemButton : UserControl
    {
        public static readonly DependencyProperty CommandProperty = DependencyProperty.Register("Command", typeof(ICommand), typeof(SystemButton));
        public ICommand Command
        {
            get { return (ICommand)GetValue(CommandProperty); }
            set { SetValue(CommandProperty, value); }
        }

        public static readonly DependencyProperty IconProperty = DependencyProperty.Register("Icon", typeof(ImageSource), typeof(SystemButton));
        public ImageSource Icon
        {
            get { return GetValue(IconProperty) as ImageSource; }
            set { SetValue(IconProperty, value); }
        }

        public static readonly DependencyProperty IconHeightProperty = DependencyProperty.Register("IconHeight", typeof(int), typeof(SystemButton), new FrameworkPropertyMetadata(10));
        public int IconHeight
        {
            get { return (int)GetValue(IconHeightProperty); }
            set { SetValue(IconHeightProperty, value); }
        }

        public static readonly DependencyProperty HoverHackgroundProperty = DependencyProperty.Register("HoverBackground", typeof(SolidColorBrush), typeof(SystemButton), new FrameworkPropertyMetadata(new SolidColorBrush(Colors.Transparent)));
        public SolidColorBrush HoverBackground
        {
            get { return GetValue(HoverHackgroundProperty) as SolidColorBrush; }
            set { SetValue(HoverHackgroundProperty, value); }
        }

        public static readonly DependencyProperty IconWidthProperty = DependencyProperty.Register("IconWidth", typeof(int), typeof(SystemButton), new FrameworkPropertyMetadata(10));
        public int IconWidth
        {
            get { return (int)GetValue(IconWidthProperty); }
            set { SetValue(IconWidthProperty, value); }
        }

        public SystemButton()
        {
            InitializeComponent();
        }

        public event RoutedEventHandler Click
        {
            add { button.AddHandler(ButtonBase.ClickEvent, value); }
            remove { button.RemoveHandler(ButtonBase.ClickEvent, value); }
        }
    }
}
