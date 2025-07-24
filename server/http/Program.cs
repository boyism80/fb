namespace Http
{
    /// <summary>
    /// Provides the entry point for the HTTP service application.
    /// </summary>
    public class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// Configures and starts the web application.
        /// </summary>
        /// <param name="args">Command line arguments array</param>
        public static void Main(string[] args)
        {
            var builder = WebApplication.CreateBuilder(args);
            var app = builder.Build();

            app.MapGet("/", () => "Hello World!");

            app.Run();
        }
    }
}