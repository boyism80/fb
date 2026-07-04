using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Media;

namespace Runner.ViewModel
{
    public enum InfraCheckStatus
    {
        Pending,
        Checking,
        Pass,
        Fail,
        Warning
    }

    public enum InfraCheckGroup
    {
        Repository,
        Toolchain,
        NativeArtifacts,
        Runtime
    }

    public class InfraCheckItem : INotifyPropertyChanged
    {
        public string Id { get; }
        public InfraCheckGroup Group { get; }

        private string _name = string.Empty;
        public string Name
        {
            get => _name;
            set { _name = value; OnPropertyChanged(); }
        }

        private string _detail = string.Empty;
        public string Detail
        {
            get => _detail;
            set { _detail = value; OnPropertyChanged(); }
        }

        private InfraCheckStatus _status = InfraCheckStatus.Pending;
        public InfraCheckStatus Status
        {
            get => _status;
            set
            {
                _status = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(StatusGlyph));
                OnPropertyChanged(nameof(StatusBrushKey));
                OnPropertyChanged(nameof(StatusBrush));
            }
        }

        private string _message = string.Empty;
        public string Message
        {
            get => _message;
            set { _message = value; OnPropertyChanged(); }
        }

        public bool Required { get; init; } = true;

        public InfraCheckItem(string id, string name, InfraCheckGroup group, bool required = true)
        {
            Id = id;
            Name = name;
            Group = group;
            Required = required;
        }

        public string StatusGlyph => Status switch
        {
            InfraCheckStatus.Pass => "\u2713",
            InfraCheckStatus.Fail => "\u2717",
            InfraCheckStatus.Warning => "\u26A0",
            InfraCheckStatus.Checking => "\u2026",
            _ => "\u00B7"
        };

        public string StatusBrushKey => Status switch
        {
            InfraCheckStatus.Pass => "PositiveBrush",
            InfraCheckStatus.Fail => "ErrorBrush",
            InfraCheckStatus.Warning => "WarningBrush",
            InfraCheckStatus.Checking => "MainTextBrush",
            _ => "SemiTextBrush"
        };

        public Brush StatusBrush =>
            Application.Current.TryFindResource(StatusBrushKey) as Brush ?? Brushes.Gray;

        public InfraCheckItem Pass(string? detail = null)
        {
            if (detail != null)
                Detail = detail;
            Status = InfraCheckStatus.Pass;
            Message = string.Empty;
            return this;
        }

        public InfraCheckItem Fail(string message, string? detail = null)
        {
            if (detail != null)
                Detail = detail;
            Status = InfraCheckStatus.Fail;
            Message = message;
            return this;
        }

        public event PropertyChangedEventHandler? PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string? propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class InfraCheckResult
    {
        public IReadOnlyList<InfraCheckItem> Items { get; init; } = Array.Empty<InfraCheckItem>();
        public int RequiredTotal { get; init; }
        public int RequiredPassed { get; init; }
        public bool CanBuild => RequiredPassed >= RequiredTotal && RequiredTotal > 0;

        public IEnumerable<InfraCheckItem> Failures =>
            Items.Where(i => i.Required && i.Status == InfraCheckStatus.Fail);
    }
}
