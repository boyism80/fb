using System.IO;
using NPOI.SS.UserModel;
using NPOI.XSSF.UserModel;

namespace MapEditor.Table
{
    /// <summary>
    /// Shared xlsx helpers. Table sheets have names on row 1, types on row 2, scope on row 3 and data from row 4.
    /// </summary>
    public static class XlsxFile
    {
        public const int FirstDataRow = 3;

        public static XSSFWorkbook Open(string path)
        {
            using var stream = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
            return new XSSFWorkbook(stream);
        }

        /// <summary>
        /// Writes through a temp file so a failed write never truncates the table.
        /// Throws IOException when Excel holds the file open.
        /// </summary>
        public static void Save(XSSFWorkbook workbook, string path)
        {
            EnsureWritable(path);

            var temp = path + ".tmp";
            using (var stream = new FileStream(temp, FileMode.Create, FileAccess.Write))
                workbook.Write(stream, leaveOpen: true);

            File.Copy(temp, path, overwrite: true);
            File.Delete(temp);
        }

        /// <summary>
        /// Throws IOException when another program (Excel) holds the file open.
        /// </summary>
        public static void EnsureWritable(string path)
        {
            using (new FileStream(path, FileMode.Open, FileAccess.ReadWrite, FileShare.None))
            { }
        }

        public static string Text(IRow row, int column)
        {
            var cell = row?.GetCell(column);
            if (cell == null)
                return "";

            var type = cell.CellType == CellType.Formula ? cell.CachedFormulaResultType : cell.CellType;
            if (type == CellType.Numeric)
                return cell.NumericCellValue.ToString(System.Globalization.CultureInfo.InvariantCulture);
            else if (type == CellType.String)
                return cell.StringCellValue ?? "";
            else if (type == CellType.Boolean)
                return cell.BooleanCellValue ? "true" : "false";
            else
                return "";
        }

        /// <summary>
        /// Writes integers as numbers when the cell was numeric before, otherwise as text like the existing sheets.
        /// </summary>
        public static void SetText(IRow row, int column, string value, bool numeric = false)
        {
            if (string.IsNullOrEmpty(value))
            {
                var existing = row.GetCell(column);
                if (existing != null)
                    row.RemoveCell(existing);
                return;
            }

            // NPOI writes a new value of an inline string cell (t="inlineStr", how the tables are stored) to <v> while
            // readers use <is>, so the old text would stay. A fresh cell with the old style avoids that.
            var old = row.GetCell(column);
            var style = old?.CellStyle;
            if (old != null)
                row.RemoveCell(old);

            var cell = row.CreateCell(column);
            if (style != null)
                cell.CellStyle = style;
            if (numeric && double.TryParse(value, out var number))
                cell.SetCellValue(number);
            else
                cell.SetCellValue(value);
        }
    }
}
