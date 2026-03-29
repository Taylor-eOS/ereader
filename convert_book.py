import unicodedata
from pathlib import Path
from collections import Counter

INPUT_FILE = "input.txt"
CLEANED_FILE = "cleaned.txt"
OUTPUT_BIN_FILE = "book.bin"
SYMBOL_TABLE_FILE = "symbol_table.txt"
ALLOWED_CHARS = [" ", "\n", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "æ", "ø", "å", "Æ", "Ø", "Å", "ä", "ö", "ü", "Ä", "Ö", "Ü", "ß", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", ",", ":", ";", "!", "?", "'", "\"", "-", "_", "(", ")", "/", "&", "*", "$", "£", "[", "]", "<", ">", "{", "}", "%", "~", "#", "+", "=", "@", "\\", "^", "|", "°", "±", "©", "®", "¶", "¬", "¦", "¢", "¥", "¤", "¡", "¿"]
DIRECT_REPLACEMENTS = {"\r\n": "\n", "\r": "\n", "\t": " ", "\u00A0": " ", "\u2000": " ", "\u2001": " ", "\u2002": " ", "\u2003": " ", "\u2004": " ", "\u2005": " ", "\u2006": " ", "\u2007": " ", "\u2008": " ", "\u2009": " ", "\u200A": " ", "\u202F": " ", "\u205F": " ", "\u3000": " ", "\u200B": "", "\u200C": "", "\u200D": "", "\uFEFF": "", "\u00AD": "", "\u2028": "\n", "\u2029": "\n", "\u0060": "'", "\u00B4": "'", "\u2018": "'", "\u2019": "'", "\u201A": "'", "\u201B": "'", "\u2039": "'", "\u203A": "'", "\u201C": "\"", "\u201D": "\"", "\u201E": "\"", "\u201F": "\"", "\u00AB": "\"", "\u00BB": "\"", "\u2014": "-", "\u2013": "-", "\u2212": "-", "\u2010": "-", "\u2011": "-", "\u2012": "-", "\u2015": "-", "\u2026": "...", "\u00B7": ".", "\u2022": "*", "\u00A3": "£", "\u00A7": "$", "\u00D7": "*", "\u2044": "/", "\u00F0": "d", "\u00D0": "D", "\u00FE": "th", "\u00DE": "Th", "\u00F7": "/", "\u03BC": "u", "\u0153": "oe", "\u0152": "OE", "\u0142": "l", "\u0141": "L", "\u0111": "d", "\u0110": "D", "\u0127": "h", "\u0126": "H", "\u0167": "t", "\u0166": "T", "\u0180": "b", "\u0243": "B", "\u0268": "i", "\u0197": "I", "\u0289": "u", "\u0244": "U", "\u0180": "b", "\u0182": "B", "\u023c": "c", "\u0188": "c", "\u0187": "C", "\u0256": "d", "\u0257": "d", "\u0493": "g", "\u0492": "G", "\u0266": "h", "\u0267": "h", "\u029d": "j", "\u0199": "k", "\u0198": "K", "\u026b": "l", "\u019a": "l", "\u0140": "l", "\u013f": "L", "\u0271": "m", "\u019d": "N", "\u0273": "n", "\u0275": "o", "\u01eb": "o", "\u2c63": "P", "\u1d7d": "p", "\u024d": "r", "\u024c": "R", "\u023f": "s", "\u0240": "z", "\u0288": "t", "\u0163": "t", "\u0162": "T", "\u0289": "u", "\u028b": "v", "\u1d8c": "v", "\u0263": "g", "\u0260": "g", "\u0262": "G", "\u00A8": "\"", "\u00AA": "a", "\u00AF": "-", "\u00B2": "2", "\u00B3": "3", "\u00B5": "u", "\u00B8": ",", "\u00B9": "1", "\u00BA": "o", "\u00BC": "1/4", "\u00BD": "1/2", "\u00BE": "3/4"}
SYMBOL_TO_ID = {char: idx for idx, char in enumerate(ALLOWED_CHARS)}
ALLOWED_SET = set(ALLOWED_CHARS)
PRECOMPOSED = set("æøåÆØÅäöüÄÖÜß")

def normalize_text(text: str) -> str:
    for src, dst in DIRECT_REPLACEMENTS.items():
        text = text.replace(src, dst)
    result = []
    for c in text:
        if c in ALLOWED_SET or c in PRECOMPOSED:
            result.append(c)
            continue
        decomposed = unicodedata.normalize("NFD", c)
        base = decomposed[0]
        if base in ALLOWED_SET and all(unicodedata.combining(d) for d in decomposed[1:]):
            result.append(base)
        else:
            result.append(c)
    return "".join(result)

def collapse_whitespace(text: str) -> str:
    lines = []
    for raw_line in text.split("\n"):
        parts = raw_line.split()
        lines.append(" ".join(parts))
    text = "\n".join(lines)
    while "\n\n\n" in text:
        text = text.replace("\n\n\n", "\n\n")
    return text.strip() + "\n"

def filter_text(text: str):
    cleaned_chars = []
    unknown_counter = Counter()
    for c in text:
        if c in ALLOWED_SET:
            cleaned_chars.append(c)
        else:
            unknown_counter[c] += 1
    return "".join(cleaned_chars), unknown_counter

def encode_symbol_stream(text: str) -> bytes:
    return bytes(SYMBOL_TO_ID[c] for c in text)

def write_symbol_table(path: Path):
    with path.open("w", encoding="utf-8") as f:
        for char, idx in SYMBOL_TO_ID.items():
            printable = "\\n" if char == "\n" else char
            f.write(f"{idx:3d}  {repr(printable)}\n")

def print_unknowns(unknown_counter: Counter):
    if not unknown_counter:
        print("No unknown characters found.")
        return
    print("Unknown characters removed:")
    for char, count in sorted(unknown_counter.items(), key=lambda x: (-x[1], x[0])):
        name = unicodedata.name(char, "UNKNOWN")
        print(f"  {repr(char)}  x{count}  U+{ord(char):04X}  {name}")

def main():
    input_path = Path(INPUT_FILE)
    cleaned_path = Path(CLEANED_FILE)
    output_bin_path = Path(OUTPUT_BIN_FILE)
    symbol_table_path = Path(SYMBOL_TABLE_FILE)
    if not input_path.exists():
        raise FileNotFoundError(f"Missing {INPUT_FILE}")
    raw_text = input_path.read_text(encoding="utf-8")
    normalized = normalize_text(raw_text)
    collapsed = collapse_whitespace(normalized)
    cleaned, unknown_counter = filter_text(collapsed)
    symbol_stream = encode_symbol_stream(cleaned)
    cleaned_path.write_text(cleaned, encoding="utf-8")
    output_bin_path.write_bytes(symbol_stream)
    write_symbol_table(symbol_table_path)
    print(f"Input chars:    {len(raw_text)}")
    print(f"Cleaned chars:  {len(cleaned)}")
    print(f"Symbol bytes:   {len(symbol_stream)}")
    print(f"Wrote: {cleaned_path}")
    print(f"Wrote: {output_bin_path}")
    print(f"Wrote: {symbol_table_path}")
    print_unknowns(unknown_counter)

if __name__ == "__main__":
    main()

