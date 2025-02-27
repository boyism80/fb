namespace Runner.ViewModel
{
    public class Point
    {
        public Model.Point Model { get; set; }
        public int X
        {
            get => Model.X;
            set => Model.X = value;
        }

        public int Y
        {
            get => Model.Y;
            set => Model.Y = value;
        }

        public Point(Model.Point model)
        {
            Model = model;
        }
    }
}
