using System.Text.RegularExpressions;

namespace AdminTool.Services
{
    public sealed class ScriptDependencyAnalysisResult
    {
        public bool Success { get; init; }
        public string Error { get; init; }
        public IReadOnlyList<string> OrderedPaths { get; init; } = Array.Empty<string>();
        public IReadOnlyDictionary<string, IReadOnlyList<string>> Dependencies { get; init; }
            = new Dictionary<string, IReadOnlyList<string>>();
    }

    public static class ScriptDependencyAnalyzer
    {
        private static readonly Regex RequireRegex = new(
            @"require\s*\(\s*['""]([^'""]+)['""]\s*\)",
            RegexOptions.Compiled | RegexOptions.CultureInvariant);

        public static ScriptDependencyAnalysisResult Analyze(
            IReadOnlyList<(string RelativePath, string Source)> entries)
        {
            if (entries == null || entries.Count == 0)
            {
                return new ScriptDependencyAnalysisResult
                {
                    Success = false,
                    Error = "분석할 스크립트가 없습니다."
                };
            }

            var byPath = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            foreach (var (relativePath, source) in entries)
            {
                var path = NormalizeRelativePath(relativePath);
                if (string.IsNullOrEmpty(path))
                {
                    return new ScriptDependencyAnalysisResult
                    {
                        Success = false,
                        Error = "상대 경로가 비어 있는 스크립트가 있습니다."
                    };
                }

                if (!byPath.TryAdd(path, source ?? string.Empty))
                {
                    return new ScriptDependencyAnalysisResult
                    {
                        Success = false,
                        Error = $"중복 경로입니다: {path}"
                    };
                }
            }

            var deps = new Dictionary<string, HashSet<string>>(StringComparer.OrdinalIgnoreCase);
            var incoming = new Dictionary<string, int>(StringComparer.OrdinalIgnoreCase);
            foreach (var path in byPath.Keys)
            {
                deps[path] = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                incoming[path] = 0;
            }

            foreach (var (path, source) in byPath)
            {
                foreach (Match match in RequireRegex.Matches(source))
                {
                    var module = match.Groups[1].Value.Trim();
                    var depPath = ModuleToRelativePath(module);
                    if (depPath == null)
                        continue;
                    if (!byPath.ContainsKey(depPath))
                        continue;
                    if (string.Equals(depPath, path, StringComparison.OrdinalIgnoreCase))
                        continue;
                    if (deps[path].Add(depPath))
                        incoming[path]++;
                }
            }

            // Kahn: edge dep -> dependent means dep first.
            // deps[path] stores dependencies of path; reverse for queue.
            var dependents = new Dictionary<string, List<string>>(StringComparer.OrdinalIgnoreCase);
            foreach (var path in byPath.Keys)
                dependents[path] = new List<string>();

            foreach (var (path, pathDeps) in deps)
            {
                foreach (var dep in pathDeps)
                    dependents[dep].Add(path);
            }

            var ready = byPath.Keys
                .Where(p => incoming[p] == 0)
                .OrderBy(p => p, StringComparer.OrdinalIgnoreCase)
                .ToList();

            var ordered = new List<string>();
            while (ready.Count > 0)
            {
                var next = ready[0];
                ready.RemoveAt(0);
                ordered.Add(next);

                foreach (var child in dependents[next].OrderBy(x => x, StringComparer.OrdinalIgnoreCase))
                {
                    incoming[child]--;
                    if (incoming[child] == 0)
                    {
                        ready.Add(child);
                        ready.Sort(StringComparer.OrdinalIgnoreCase);
                    }
                }
            }

            if (ordered.Count != byPath.Count)
            {
                var remaining = byPath.Keys
                    .Where(p => !ordered.Contains(p, StringComparer.OrdinalIgnoreCase))
                    .OrderBy(p => p, StringComparer.OrdinalIgnoreCase);
                return new ScriptDependencyAnalysisResult
                {
                    Success = false,
                    Error = $"순환 참조가 감지되었습니다: {string.Join(" -> ", remaining)}",
                    Dependencies = deps.ToDictionary(
                        kv => kv.Key,
                        kv => (IReadOnlyList<string>)kv.Value.OrderBy(x => x, StringComparer.OrdinalIgnoreCase).ToList(),
                        StringComparer.OrdinalIgnoreCase)
                };
            }

            return new ScriptDependencyAnalysisResult
            {
                Success = true,
                OrderedPaths = ordered,
                Dependencies = deps.ToDictionary(
                    kv => kv.Key,
                    kv => (IReadOnlyList<string>)kv.Value.OrderBy(x => x, StringComparer.OrdinalIgnoreCase).ToList(),
                    StringComparer.OrdinalIgnoreCase)
            };
        }

        public static string NormalizeRelativePath(string path)
        {
            if (string.IsNullOrWhiteSpace(path))
                return null;

            var normalized = path.Trim().Replace('\\', '/');
            while (normalized.StartsWith('/'))
                normalized = normalized[1..];
            if (normalized.StartsWith("scripts/", StringComparison.OrdinalIgnoreCase))
                normalized = normalized["scripts/".Length..];
            return normalized;
        }

        public static string ModuleToRelativePath(string module)
        {
            if (string.IsNullOrWhiteSpace(module))
                return null;

            var name = module.Trim().Replace('\\', '/');
            if (!name.StartsWith("lib.", StringComparison.OrdinalIgnoreCase) &&
                !name.StartsWith("lib/", StringComparison.OrdinalIgnoreCase))
            {
                return null;
            }

            if (name.StartsWith("lib.", StringComparison.OrdinalIgnoreCase))
                name = "lib/" + name["lib.".Length..].Replace('.', '/');
            if (!name.EndsWith(".lua", StringComparison.OrdinalIgnoreCase))
                name += ".lua";
            return NormalizeRelativePath(name);
        }
    }
}
