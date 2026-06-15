namespace Http.Filters
{
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Method, Inherited = true, AllowMultiple = false)]
    public sealed class SuppressRequestLogAttribute : Attribute
    {
    }
}
