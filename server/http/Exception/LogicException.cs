using Fb.Model.EnumValue;

namespace Http
{
    /// <summary>
    /// Represents an exception that occurs due to business logic violations or game rule errors.
    /// Provides structured error handling with specific error codes for different logic failures.
    /// </summary>
    public class LogicException : Exception
    {
        /// <summary>
        /// Gets the specific error code that identifies the type of logic error that occurred.
        /// </summary>
        /// <value>The error code enum value representing the specific logic failure.</value>
        public ErrorCode Error { get; private set; }

        /// <summary>
        /// Gets a message that describes the current exception with the error code information.
        /// </summary>
        /// <value>A formatted string containing the exception type and error code.</value>
        public override string Message => $"LogicException code : {Error}";

        /// <summary>
        /// Initializes a new instance of the <see cref="LogicException"/> class with the specified error code.
        /// </summary>
        /// <param name="error">The error code that identifies the specific logic failure.</param>
        public LogicException(ErrorCode error)
        {
            Error = error;
        }
    }
}