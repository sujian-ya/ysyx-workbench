#include <elf.h>
#include <ftrace.h>
#include <common.h>

#ifdef FTRACE_COND
#define STACK_DEPTH 1024
// 定义函数调用栈，现在存储的是被调用函数的入口地址
static vaddr_t call_stack[STACK_DEPTH];
// 记录函数调用栈的深度
static int stack_ptr = -1; // -1 表示栈空

#define SYM_NUM 1024
static struct {
    vaddr_t addr;
    char name[64];
} sym_table[SYM_NUM];
static int sym_count = 0;
#endif

long load_elf(const char *elf_file) {
    if (!elf_file) {
        Log("No elf is given. Use the default build-in elf.");
        return 0; // Indicate no ELF loaded
    }
    FILE *fp = fopen(elf_file, "rb");
    if (!fp) {
        log_write("Can not open '%s'", elf_file);
        return 0; // Indicate failure to open
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    Log("The elf is %s, size = %ld", elf_file, size);

    fclose(fp);
    return 1; // Indicate success
}

void init_elf(const char *elf_file) {
    // This initial call to load_elf is redundant as init_elf will re-open and parse.
    // It's mostly for logging/initial check.
    long load_ret = load_elf(elf_file);
    if (load_ret == 0) {
        return;
    }

#ifdef FTRACE_COND
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
    // Basic ELF magic number check
    if (ehdr.e_ident[EI_MAG0] != ELFMAG0 || ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr.e_ident[EI_MAG2] != ELFMAG2 || ehdr.e_ident[EI_MAG3] != ELFMAG3) {
        log_write("Not a valid ELF file: %s\n", elf_file);
        fclose(fp);
        return;
    }

    // Read all section headers
    // Using a VLA (Variable Length Array) which is a C99 feature.
    // For C89/C++ compatibility, dynamic allocation would be needed.
    Elf32_Shdr shdrs[ehdr.e_shnum];
    fseek(fp, ehdr.e_shoff, SEEK_SET);
    if (fread(shdrs, sizeof(Elf32_Shdr), ehdr.e_shnum, fp) != ehdr.e_shnum) {
        log_write("Failed to read section headers from: %s\n", elf_file);
        fclose(fp);
        return;
    }

    // Find symbol table section and its corresponding string table section
    Elf32_Shdr *symtab_sh = NULL;
    Elf32_Shdr *strtab_sh = NULL;
    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (shdrs[i].sh_type == SHT_SYMTAB) {
            symtab_sh = &shdrs[i];
            // sh_link points to the index of the string table section
            if (symtab_sh->sh_link < ehdr.e_shnum) {
                strtab_sh = &shdrs[symtab_sh->sh_link];
            }
            break; // Found SYMTAB, no need to continue
        }
    }
    if (!symtab_sh || !strtab_sh) {
        log_write("No symbol table or string table found in: %s\n", elf_file);
        fclose(fp);
        return;
    }

    // Read string table into memory
    char *strtab = (char *)malloc(strtab_sh->sh_size);
    if (!strtab) {
        log_write("Failed to allocate memory for string table\n");
        fclose(fp);
        return;
    }
    fseek(fp, strtab_sh->sh_offset, SEEK_SET);
    if (fread(strtab, 1, strtab_sh->sh_size, fp) != strtab_sh->sh_size) {
        log_write("Failed to read string table from: %s\n", elf_file);
        free(strtab); // Free allocated memory on failure
        fclose(fp);
        return;
    }

    // Read symbol table and parse function symbols
    int nsyms = symtab_sh->sh_size / sizeof(Elf32_Sym);
    fseek(fp, symtab_sh->sh_offset, SEEK_SET);
    for (int i = 0; i < nsyms; i++) {
        Elf32_Sym sym;
        if (fread(&sym, sizeof(Elf32_Sym), 1, fp) != 1) {
            log_write("Failed to read symbol at index %d from: %s\n", i, elf_file);
            break; // Stop on read error
        }
        // Only process function type symbols that have a name and fit in our table
        if (ELF32_ST_TYPE(sym.st_info) == STT_FUNC && sym.st_name != 0 && sym_count < SYM_NUM) {
            strncpy(sym_table[sym_count].name, strtab + sym.st_name, sizeof(sym_table[0].name) - 1);
            sym_table[sym_count].name[sizeof(sym_table[0].name) - 1] = '\0'; // Ensure null-termination
            sym_table[sym_count].addr = sym.st_value;
            sym_count++;
        }
    }

    free(strtab); // Free allocated string table memory
    fclose(fp);   // Close the ELF file
    return;
#endif
}

#ifdef FTRACE_COND
// 根据地址获取函数名。现在addr应该是一个函数入口地址，所以我们期望精确匹配。
static const char *get_func_name(vaddr_t addr) {
    // 遍历符号表查找匹配的函数地址
    for (int i = 0; i < sym_count; i++) {
        if (sym_table[i].addr == addr) {
            return sym_table[i].name;
        }
    }
    // 如果没有找到精确匹配，返回未知
    return "???";
}
#endif

void ftrace_call(vaddr_t pc, vaddr_t target) {
#ifdef FTRACE_COND
    // 检查栈是否会溢出
    if (stack_ptr + 1 >= STACK_DEPTH) {
        log_write("Function call stack overflow! (PC: 0x%08x, Target: 0x%08x)\n", pc, target);
        // 在溢出时，不进行压栈操作，直接返回。
        return;
    }
    
    // 计算即将压栈后的深度所对应的缩进级别
    // stack_ptr 是当前栈深度 - 1。新的深度是 stack_ptr + 1。
    char indent[(stack_ptr + 1) * 2 + 1];
    memset(indent, ' ', (stack_ptr + 1) * 2);
    indent[(stack_ptr + 1) * 2] = '\0';
    
    // 获取目标函数名称
    const char *func_name = get_func_name(target);
    // target 就是函数的入口地址
    vaddr_t func_addr = target; 
    
    // 打印调用日志
    log_write("0x%08x: %scall [%s@0x%08x+0x%x]\n", pc, indent, func_name, func_addr, target - func_addr);

    // 压入目标函数的入口地址，然后更新栈指针
    stack_ptr++;
    call_stack[stack_ptr] = target;
    return;
#endif
}

void ftrace_ret(vaddr_t pc, vaddr_t ret_addr) {
#ifdef FTRACE_COND
    // 检查栈是否会下溢
    if (stack_ptr < 0) {
        log_write("Function call stack underflow! (PC: 0x%08x, Return_Target: 0x%08x)\n", pc, ret_addr);
        return;
    }
    
    // 获取当前栈顶存储的函数入口地址
    vaddr_t func_entry_addr_on_stack = call_stack[stack_ptr];

    // 获取函数名称
    const char *func_name = get_func_name(func_entry_addr_on_stack);
    // 函数地址就是栈上存储的入口地址
    vaddr_t func_addr = func_entry_addr_on_stack; 

    // 计算缩进，使用当前栈深度（出栈前的深度）
    char indent[stack_ptr * 2 + 1];
    memset(indent, ' ', stack_ptr * 2);
    indent[stack_ptr * 2] = '\0';
    
    // 打印返回日志
    log_write("0x%08x: %sret  [%s@0x%08x]\n", pc, indent, func_name, func_addr);

    // 出栈
    stack_ptr--;
#endif
}