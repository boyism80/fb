using System.ComponentModel;

namespace MapEditor.Table
{
    /// <summary>
    /// Base of the spawn rows. PropertyChanged.Fody calls OnPropertyChanged with the old and new values, which the
    /// document records for undo. Values derived from other properties are not recorded.
    /// </summary>
    public abstract class Entity : INotifyPropertyChanged
    {
        private static readonly HashSet<string> Derived = new HashSet<string> { "Info", "Name", "DestMap", "DestX", "DestY", "DestName", "DestLabel", "DirectionIndex", "Left", "Top", "Right", "Bottom" };

        public event PropertyChangedEventHandler PropertyChanged;
        public event Action<Entity, string, object, object> Edited;

        public void OnPropertyChanged(string propertyName, object before, object after)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
            if (Derived.Contains(propertyName) == false && Equals(before, after) == false)
                Edited?.Invoke(this, propertyName, before, after);
        }
    }
}
