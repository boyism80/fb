#!/usr/bin/env python3
"""
Analyze std::function parameter usage in C++ codebase.
Excludes dependency directory and auto-generated files.
"""

import os
import re
from pathlib import Path
from collections import defaultdict
from typing import List, Dict, Tuple

# Patterns to exclude
EXCLUDE_DIRS = ['dependency', 'build', 'CMakeFiles', '__pycache__', 'x64', 'bin', '.git', 'tools']
EXCLUDE_PATTERNS = [
    r'include/fb/model/.*',  # Auto-generated model files
    r'include/protocol/flatbuffer/.*',  # Auto-generated FlatBuffer protocol files
    r'include/fb/protocol/flatbuffer/.*',  # Auto-generated FlatBuffer protocol files
    r'include/fb/login/protocol/flatbuffer/.*',  # Auto-generated FlatBuffer protocol files
]

# Pattern to match std::function in function parameters
FUNCTION_PATTERN = re.compile(
    r'(?:const\s+)?std::function\s*<[^>]+>\s*(?:&&|&|\*)?', 
    re.MULTILINE
)

# Pattern to match function declarations with std::function parameters
FUNCTION_DECL_PATTERN = re.compile(
    r'(?:^|\n)\s*(?:template\s*<[^>]+>\s*)?'  # Optional template
    r'(?:inline\s+|static\s+|virtual\s+|explicit\s+)?'  # Optional keywords
    r'(?:[a-zA-Z_][a-zA-Z0-9_<>:,\s&*]*\s+)?'  # Return type
    r'([a-zA-Z_][a-zA-Z0-9_]*)\s*'  # Function name
    r'\([^)]*\)',  # Parameters
    re.MULTILINE
)

def should_exclude_file(filepath: Path) -> bool:
    """Check if file should be excluded."""
    path_str = str(filepath)
    
    # Check exclude directories
    for exclude_dir in EXCLUDE_DIRS:
        if exclude_dir in path_str.split(os.sep):
            return True
    
    # Check exclude patterns
    for pattern in EXCLUDE_PATTERNS:
        if re.match(pattern, path_str.replace('\\', '/')):
            return True
    
    return False

def find_function_parameters(content: str, line_num: int = 0) -> List[Dict]:
    """Find all std::function parameters in function declarations."""
    results = []
    
    # Find all std::function occurrences
    for match in FUNCTION_PATTERN.finditer(content):
        start_pos = match.start()
        line_number = content[:start_pos].count('\n') + 1 + line_num
        
        # Try to find the function declaration context
        # Look backwards for function name
        before = content[:start_pos]
        after = content[start_pos:]
        
        # Extract the parameter type
        param_match = match.group(0)
        
        # Determine parameter passing style
        param_style = 'unknown'
        if 'const' in param_match and '&' in param_match:
            param_style = 'const_ref'
        elif '&&' in param_match:
            param_style = 'rvalue_ref'
        elif '&' in param_match:
            param_style = 'ref'
        elif '*' in param_match:
            param_style = 'pointer'
        else:
            param_style = 'by_value'
        
        # Try to find function name
        func_name = 'unknown'
        # Look for function name before this position
        func_match = re.search(r'([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*' + re.escape(param_match), before)
        if func_match:
            func_name = func_match.group(1)
        else:
            # Try to find in the line
            line_start = before.rfind('\n')
            line_end = after.find('\n')
            if line_end == -1:
                line_end = len(after)
            line = before[line_start+1:] + after[:line_end]
            func_match = re.search(r'([a-zA-Z_][a-zA-Z0-9_]*)\s*\(', line)
            if func_match:
                func_name = func_match.group(1)
        
        results.append({
            'line': line_number,
            'param_type': param_match.strip(),
            'style': param_style,
            'function': func_name,
            'context': line[:200] if len(line) > 200 else line
        })
    
    return results

def analyze_file(filepath: Path) -> List[Dict]:
    """Analyze a single file for std::function parameters."""
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        results = find_function_parameters(content)
        for result in results:
            result['file'] = str(filepath)
        
        return results
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return []

def main():
    """Main analysis function."""
    project_root = Path('.')
    
    # Statistics
    stats = defaultdict(int)
    all_results = []
    file_results = defaultdict(list)
    
    # Find all C++ files
    cpp_files = []
    for ext in ['*.cpp', '*.h', '*.hpp']:
        cpp_files.extend(project_root.rglob(ext))
    
    print(f"Found {len(cpp_files)} C++ files")
    print("Analyzing...\n")
    
    # Analyze each file
    for filepath in cpp_files:
        if should_exclude_file(filepath):
            continue
        
        results = analyze_file(filepath)
        if results:
            all_results.extend(results)
            file_results[str(filepath)] = results
            for result in results:
                stats[result['style']] += 1
    
    # Print summary
    print("=" * 80)
    print("SUMMARY")
    print("=" * 80)
    print(f"Total files analyzed: {len([f for f in cpp_files if not should_exclude_file(f)])}")
    print(f"Files with std::function parameters: {len(file_results)}")
    print(f"Total std::function parameters found: {len(all_results)}")
    print("\nParameter passing style distribution:")
    for style, count in sorted(stats.items(), key=lambda x: -x[1]):
        print(f"  {style:15s}: {count:4d}")
    
    # Print detailed results by file
    print("\n" + "=" * 80)
    print("DETAILED RESULTS BY FILE")
    print("=" * 80)
    
    for filepath, results in sorted(file_results.items()):
        print(f"\n{filepath}:")
        for result in results:
            print(f"  Line {result['line']:4d}: [{result['style']:15s}] {result['param_type']}")
            if result['function'] != 'unknown':
                print(f"           Function: {result['function']}")
    
    # Print recommendations
    print("\n" + "=" * 80)
    print("RECOMMENDATIONS")
    print("=" * 80)
    
    print("\n1. const std::function& (const reference)")
    print("   - Use when: Function is called multiple times, no need to move")
    print("   - Pros: No copy overhead, clear intent")
    print("   - Cons: Cannot move from parameter")
    print(f"   - Current usage: {stats.get('const_ref', 0)} occurrences")
    
    print("\n2. std::function&& (rvalue reference)")
    print("   - Use when: Function is stored or moved into container")
    print("   - Pros: Allows move semantics, efficient")
    print("   - Cons: Caller must use std::move()")
    print(f"   - Current usage: {stats.get('rvalue_ref', 0)} occurrences")
    
    print("\n3. std::function (by value)")
    print("   - Use when: Function is copied into member variable")
    print("   - Pros: Simple, caller can pass rvalue or lvalue")
    print("   - Cons: May cause unnecessary copies")
    print(f"   - Current usage: {stats.get('by_value', 0)} occurrences")
    
    print("\n4. std::function& (non-const reference)")
    print("   - Use when: Function needs to be modified")
    print("   - Pros: Allows modification")
    print("   - Cons: Rarely needed, can be confusing")
    print(f"   - Current usage: {stats.get('ref', 0)} occurrences")
    
    print("\n5. std::function* (pointer)")
    print("   - Use when: Function is optional (can be nullptr)")
    print("   - Pros: Can represent optional callback")
    print("   - Cons: Requires null checks")
    print(f"   - Current usage: {stats.get('pointer', 0)} occurrences")
    
    # Save detailed report
    report_file = 'std_function_analysis_report.txt'
    with open(report_file, 'w', encoding='utf-8') as f:
        f.write("STD::FUNCTION PARAMETER ANALYSIS REPORT\n")
        f.write("=" * 80 + "\n\n")
        f.write(f"Total occurrences: {len(all_results)}\n")
        f.write(f"Files analyzed: {len(file_results)}\n\n")
        
        f.write("BY PARAMETER STYLE:\n")
        for style, count in sorted(stats.items(), key=lambda x: -x[1]):
            f.write(f"  {style:15s}: {count:4d}\n")
        
        f.write("\n\nDETAILED RESULTS:\n")
        f.write("=" * 80 + "\n")
        for filepath, results in sorted(file_results.items()):
            f.write(f"\n{filepath}:\n")
            for result in results:
                f.write(f"  Line {result['line']:4d}: [{result['style']:15s}] {result['param_type']}\n")
                if result['function'] != 'unknown':
                    f.write(f"           Function: {result['function']}\n")
    
    print(f"\nDetailed report saved to: {report_file}")

if __name__ == '__main__':
    main()
