#!/usr/bin/env python3
import re
import sys

DROP_PREFIXES = (
    r"\.file\b",
    r"\.loc\b",
    r"\.cfi_",
    r"\.ident\b",
    r"\.addrsig\b",
    r"\.addrsig_sym\b",
    r"\.section\s+\.note\.GNU-stack",
    
    # COFF/PE (MinGW) decorations:
    r"\.def\b",
    r"\.endef\b",
    r"\.scl\b",
    r"\.type\b",
)

DROP_EXACT = {
    ".intel_syntax noprefix",
    ".att_syntax",
}

def is_drop(line: str) -> bool:
    s = line.strip()
    if s in DROP_EXACT:
        return True
    for p in DROP_PREFIXES:
        if re.match(p, s):
            return True
    return False

def convert_line(line: str) -> str:
    # GAS comments: '#' -> ';'
    # keep '#' inside strings unlikely in asm; simple replace is ok for typical output
    if "#" in line:
        parts = line.split("#", 1)
        line = parts[0].rstrip() + (" ;" + parts[1] if len(parts) > 1 else "") + "\n"

    s = line.strip()
    if not s:
        return line

    # .globl -> global
    m = re.match(r"\.globl\s+(\S+)", s)
    if m:
        return f"global {m.group(1)}\n"

    # .global -> global
    m = re.match(r"\.global\s+(\S+)", s)
    if m:
        return f"global {m.group(1)}\n"

    # .text/.data/.bss
    if s == ".text":
        return "section .text\n"
    if s == ".data":
        return "section .data\n"
    if s == ".bss":
        return "section .bss\n"

    # .section .rodata -> section .rodata (NASM accepts section name)
    m = re.match(r"\.section\s+([^\s,]+)", s)
    if m:
        sec = m.group(1)
        return f"section {sec}\n"

    # .align / .p2align -> align (rough)
    m = re.match(r"\.(p2align|balign|align)\s+(\d+)", s)
    if m:
        n = int(m.group(2))
        # p2align is power-of-two
        if m.group(1) == "p2align":
            return f"align {1<<n}\n"
        return f"align {n}\n"

    # .byte/.word/.long/.quad -> db/dw/dd/dq
    m = re.match(r"\.(byte|word|long|quad)\s+(.*)", s)
    if m:
        kind = m.group(1)
        rest = m.group(2)
        kw = {"byte": "db", "word": "dw", "long": "dd", "quad": "dq"}[kind]
        return f"{kw} {rest}\n"

    # .ascii/.asciz
    m = re.match(r"\.(ascii|asciz)\s+(.*)", s)
    if m:
        kind = m.group(1)
        rest = m.group(2)
        if kind == "asciz":
            return f"db {rest}, 0\n"
        return f"db {rest}\n"

    # .comm sym,size,align  -> common sym size (align ignored)
    m = re.match(r"\.comm\s+(\S+)\s*,\s*(\d+)", s)
    if m:
        return f"common {m.group(1)} {m.group(2)}\n"

    # drop .type/.size completely (ELF metadata)
    if s.startswith(".type") or s.startswith(".size"):
        return ""

    # RIP-relative style: [rip + foo] -> [rel foo]  (simple cases)
    line = re.sub(r"\[\s*rip\s*\+\s*([A-Za-z_.$][\w.$@]*)\s*\]", r"[rel \1]", line)

    # GAS local labels ".Lxyz:" -> "Lxyz:"
    line = re.sub(r"^\s*\.L([A-Za-z0-9_.$]+):", r"L\1:", line)

    # "DWORD PTR [rax]" -> "dword [rax]" (NASM prefers lowercase + no PTR)
    line = re.sub(r"\b(BYTE|WORD|DWORD|QWORD)\s+PTR\s+\[", lambda m: m.group(1).lower()+" [", line)

    # GAS/COFF: "OFFSET FLAT:symbol"  -> "symbol"
    line = re.sub(r"\bOFFSET\s+FLAT:\s*", "", line)

    # manchmal auch nur "FLAT:symbol"
    line = re.sub(r"\bFLAT:\s*", "", line)
    
    return line

def main():
    if len(sys.argv) != 3:
        print("usage: gas2nasm.py <in.s> <out.asm>", file=sys.stderr)
        sys.exit(1)

    inp, outp = sys.argv[1], sys.argv[2]
    with open(inp, "r", encoding="utf-8", errors="ignore") as f, open(outp, "w", encoding="utf-8") as g:
        for line in f:
            if is_drop(line):
                continue
            g.write(convert_line(line))

if __name__ == "__main__":
    main()
