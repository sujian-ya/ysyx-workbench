#include <ftrace.h>
#include <elf.h>
#include <common.h>

#ifdef FTRACE_COND
#define STACK_DEPTH 1024
// 定义函数调用栈
static vaddr_t call_stack[STACK_DEPTH];
// 记录函数调用栈的深度
static int stack_ptr = -1;
#endif

#define SYM_NUM 1024
static struct {
    vaddr_t addr;
    char name[64];
} sym_table[SYM_NUM];
static int sym_count = 0;

long load_elf(const char *elf_file) {
    if (!elf_file) {
        Log("No elf is given. Use the default build-in elf.");
        return 0; // built-in elf size
    }

    FILE *fp = fopen(elf_file, "rb");
    if (!fp) {
        log_write("Can not open '%s'", elf_file);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    Log("The elf is %s, size = %ld", elf_file, size);

    fclose(fp);
    return 1;
}

void init_elf(const char *elf_file) {
    long load_ret = load_elf(elf_file);
    if (load_ret == 0) {
        return;
    }

    FILE *fp = fopen(elf_file, "rb");
    if (!fp) {
        log_write("Failed to open ELF file for init: %s\n", elf_file);
        return;
    }

    Elf32_Ehdr ehdr;
    if (fread(&ehdr, sizeof(Elf32_Ehdr), 1, fp) != 1) {
        log_write("Failed to read ELF header from: %s\n", elf_file);
        fclose(fp);
        return;
    }
    if (ehdr.e_ident[EI_MAG0] != ELFMAG0 || ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr.e_ident[EI_MAG2] != ELFMAG2 || ehdr.e_ident[EI_MAG3] != ELFMAG3) {
        log_write("Not a valid ELF file: %s\n", elf_file);
        fclose(fp);
        return;
    }

    // 读取所有节头
    Elf32_Shdr shdrs[ehdr.e_shnum];
    fseek(fp, ehdr.e_shoff, SEEK_SET);
    if (fread(shdrs, sizeof(Elf32_Shdr), ehdr.e_shnum, fp) != ehdr.e_shnum) {
        log_write("Failed to read section headers from: %s\n", elf_file);
        fclose(fp);
        return;
    }

    // 找到符号表节和对应的字符串表节
    Elf32_Shdr *symtab_sh = NULL;
    Elf32_Shdr *strtab_sh = NULL;
    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (shdrs[i].sh_type == SHT_SYMTAB) {
            symtab_sh = &shdrs[i];
            // sh_link 指向字符串表节的索引
            if (symtab_sh->sh_link < ehdr.e_shnum) {
                strtab_sh = &shdrs[symtab_sh->sh_link];
            }
            break;
        }
    }
    if (!symtab_sh || !strtab_sh) {
        log_write("No symbol table or string table found in: %s\n", elf_file);
        fclose(fp);
        return;
    }

    // 读取字符串表到内存
    char *strtab = (char *)malloc(strtab_sh->sh_size);
    if (!strtab) {
        log_write("Failed to allocate memory for string table\n");
        fclose(fp);
        return;
    }
    fseek(fp, strtab_sh->sh_offset, SEEK_SET);
    if (fread(strtab, 1, strtab_sh->sh_size, fp) != strtab_sh->sh_size) {
        log_write("Failed to read string table from: %s\n", elf_file);
        free(strtab);
        fclose(fp);
        return;
    }

    // 读取符号表并解析函数符号
    int nsyms = symtab_sh->sh_size / sizeof(Elf32_Sym);
    fseek(fp, symtab_sh->sh_offset, SEEK_SET);
    for (int i = 0; i < nsyms; i++) {
        Elf32_Sym sym;
        if (fread(&sym, sizeof(Elf32_Sym), 1, fp) != 1) {
            log_write("Failed to read symbol at index %d from: %s\n", i, elf_file);
            break;
        }
        if (ELF32_ST_TYPE(sym.st_info) == STT_FUNC && sym.st_name != 0 && sym_count < SYM_NUM) {
            strncpy(sym_table[sym_count].name, strtab + sym.st_name, 63);
            sym_table[sym_count].name[63] = '\0';
            sym_table[sym_count].addr = sym.st_value;
            sym_count++;
        }
    }

    free(strtab);

    // 输出elf解析出的函数，用于调试
    // for (int i = 0; i < sym_count; i++) {
    //     printf("Symbol: %s at 0x%08x\n", sym_table[i].name, sym_table[i].addr);
    // }

    fclose(fp);
    return;
}

#ifdef FTRACE_COND
static const char *get_func_name(vaddr_t addr) {
    const char *name = "???";
    vaddr_t best = 0;
    for (int i = 0; i < sym_count; i++) {
        if (sym_table[i].addr <= addr && sym_table[i].addr >= best) {
            best = sym_table[i].addr;
            name = sym_table[i].name;
        }
    }
    return name;
}
#endif

void ftrace_call(vaddr_t pc, vaddr_t target) {
#ifdef FTRACE_COND
    stack_ptr++;
    if (stack_ptr >= STACK_DEPTH) {
        log_write("Function call stack overflow!\n");
        stack_ptr--;
        return;
    }
    call_stack[stack_ptr] = pc;
    const char *func_name = get_func_name(target);
    vaddr_t func_addr = 0;
    for (int i = 0; i < sym_count; i++) {
        if (strcmp(sym_table[i].name, func_name) == 0) {
            func_addr = sym_table[i].addr;
            break;
        }
    }
    char indent[stack_ptr * 2 + 1];
    memset(indent, ' ', stack_ptr * 2);
    indent[stack_ptr * 2] = '\0';
    log_write("0x%08x: %scall [%s@0x%08x+0x%x]\n", pc, indent, func_name, func_addr, target - func_addr);
    return;
#endif
}

void ftrace_ret(vaddr_t pc, vaddr_t ret_addr) {
#ifdef FTRACE_COND
    if (stack_ptr >= 0) {
        const char *func_name = get_func_name(call_stack[stack_ptr]);
        vaddr_t func_addr = 0;
        for (int i = 0; i < sym_count; i++) {
            if (strcmp(sym_table[i].name, func_name) == 0) {
                func_addr = sym_table[i].addr;
                break;
            }
        }
        char indent[stack_ptr * 2 + 1];
        memset(indent, ' ', stack_ptr * 2);
        indent[stack_ptr * 2] = '\0';
        log_write("0x%08x: %sret  [%s@0x%08x]\n", pc, indent, func_name, func_addr);
        stack_ptr--;
    }
#endif
}
