#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

//The simulator part of the ISA Project

//////////////////////////////////////////
//----------------Structs---------------//
//////////////////////////////////////////
typedef struct s_instruction {
    uint8_t opcode;
    uint8_t rd;
    uint8_t rs;
    uint8_t rt;
    uint8_t rm;
    int16_t immediate1;
    int16_t immediate2;
} Instruction;

typedef struct s_trace_line {
    uint16_t pc;
    Instruction inst;
    int32_t reg_pointer_array_snap[16];
} Trace_line;

typedef struct s_trace_line_node {
    struct s_trace_line_node* next;
    Trace_line trace_line;
} Trace_line_node;

typedef struct s_hw_trace_line {
    uint64_t cycle;
    uint8_t read; //0 is write, 1 is read.
    uint8_t reg_num; //as described in I/O reg table.
    uint32_t data;
} Hw_trace_line;

typedef struct s_hw_trace_line_node {
    struct s_hw_trace_line_node* next;
    Hw_trace_line hw_trace_line;
} Hw_trace_line_node;

typedef struct s_display_trace_line {
    uint64_t cycle;
    uint32_t display_snapshot;
} Display_trace_line;

typedef struct s_display_trace_line_node {
    struct s_display_trace_line_node* next;
    Display_trace_line display_trace_line;
} Display_trace_line_node;

typedef struct s_irq2_node {
    uint64_t cycle;
    struct s_irq2_node* next;
} irq2_node;

//////////////////////////////////////////
//-----------Global Varivables----------//
//////////////////////////////////////////

//------General Constants------//
uint16_t PC;
uint64_t DISK_TIMEOUT = 0;
int CURR_SIG = -1;

//------Registers------//
int32_t  R_Zero;
int32_t  R_imm1;
int32_t  R_imm2;
int32_t  R_v0;
int32_t  R_a0;
int32_t  R_a1;
int32_t  R_a2;
int32_t  R_t0;
int32_t  R_t1;
int32_t  R_t2;
int32_t  R_s0;
int32_t  R_s1;
int32_t  R_s2;
int32_t  R_gp;
int32_t  R_sp;
int32_t  R_ra;

int32_t* reg_pointer_array[] = {
    &R_Zero,
    &R_imm1,
    &R_imm2,
    &R_v0,
    &R_a0,
    &R_a1,
    &R_a2,
    &R_t0,
    &R_t1,
    &R_t2,
    &R_s0,
    &R_s1,
    &R_s2,
    &R_gp,
    &R_sp,
    &R_ra,
};

//------IORegisters------//
int32_t irq0enable;
int32_t irq1enable;
int32_t irq2enable;
int32_t irq0status;
int32_t irq1status;
int32_t irq2status;
int32_t irqhandler;
int32_t irqreturn;
int32_t clks;
int32_t leds;
int32_t display7seg;
int32_t timerenable;
int32_t timercurrent;
int32_t timermax;
int32_t diskcmd;
int32_t disksector;
int32_t diskbuffer;
int32_t diskstatus;
int32_t reserved1;
int32_t reserved2;
int32_t monitoraddr;
int32_t monitordata;
int32_t monitorcmd;

int32_t* IO_reg_pointer_array[] = {
    &irq0enable,
    &irq1enable,
    &irq2enable,
    &irq0status,
    &irq1status,
    &irq2status,
    &irqhandler,
    &irqreturn,
    &clks,
    &leds,
    &display7seg,
    &timerenable,
    &timercurrent,
    &timermax,
    &diskcmd,
    &disksector,
    &diskbuffer,
    &diskstatus,
    &reserved1,
    &reserved2,
    &monitoraddr,
    &monitordata,
    &monitorcmd
};

//------IO and Mem Contants------//
uint8_t monitor[256][256];
Instruction imemin_instructions_array[4096];
uint32_t dmem_array[4096];
uint32_t disk_matrix[128][128];

//-------Tracing Lists----------//
Trace_line_node* head_trace_line_list = NULL;
Trace_line_node* curr_trace_line_node = NULL;
Hw_trace_line_node* head_hw_trace_line_list = NULL;
Hw_trace_line_node* curr_hw_trace_line_node = NULL;
Display_trace_line_node* head_leds_trace_list = NULL;
Display_trace_line_node* curr_leds_trace_node = NULL;
Display_trace_line_node* head_dis7seg_trace_list = NULL;
Display_trace_line_node* curr_dis7seg_trace_node = NULL;
irq2_node* head_irq2_list = NULL;
irq2_node* curr_irq2_node = NULL;

const char* IO_reg_names[] = {
    "irq0enable",
    "irq1enable",
    "irq2enable",
    "irq0status",
    "irq1status",
    "irq2status",
    "irqhandler",
    "irqreturn",
    "clks",
    "leds",
    "display7seg",
    "timerenable",
    "timercurrent",
    "timermax",
    "diskcmd",
    "disksector",
    "diskbuffer",
    "diskstatus",
    "reserved",
    "reserved",
    "monitoraddr",
    "monitordata",
    "monitorcmd"
};

uint64_t cycle_counter;

//file paths
char* imemin_path;
char* dmemin_path;
char* diskin_path;
char* irq2in_path;
char* dmemout_path;
char* regout_path;
char* trace_path;
char* hw_reg_trace_path;
char* cycles_path;
char* leds_path;
char* display7seg_path;
char* diskout_path;
char* monitor_txt_path;
char* monitor_yuv_path;

//////////////////////////////////////////
//-----------Simulator Functions--------//
//////////////////////////////////////////

//-----------Init functions-------------//
//reads from imemin.txt the instructions, and inits the "imemin_instructions_array" 
int neg_mask = 61440;
int Init_instructions_array() {
    unsigned int i = 0;
    char ThreeCharString[4];  // Including the null terminator
    char TwoCharString[3];  // Including the null terminator
    char OneCharString[2];  // Including the null terminator
    unsigned int result;
    FILE* file_r = fopen(imemin_path, "r");
    char buffer[14]; //cause every line is 13 chars (including \n)
    if (file_r == NULL) { fprintf(stderr, "could not open path %s in r mode\n", imemin_path); return 0; };
    while (fgets(buffer, sizeof(buffer), file_r) != NULL && i < 4096) {
        strncpy(TwoCharString, buffer, 2); //opcode
        TwoCharString[2] = '\0';
        result = strtoul(TwoCharString, NULL, 16);
        imemin_instructions_array[i].opcode = (uint8_t)result;
        strncpy(OneCharString, buffer + 2, 1); //rd
        OneCharString[1] = '\0';
        result = strtoul(OneCharString, NULL, 16);
        imemin_instructions_array[i].rd = (uint8_t)result;
        strncpy(OneCharString, buffer + 3, 1); //rs
        OneCharString[1] = '\0';
        result = strtoul(OneCharString, NULL, 16);
        imemin_instructions_array[i].rs = (uint8_t)result;
        strncpy(OneCharString, buffer + 4, 1); //rt
        OneCharString[1] = '\0';
        result = strtoul(OneCharString, NULL, 16);
        imemin_instructions_array[i].rt = (uint8_t)result;
        strncpy(OneCharString, buffer + 5, 1); //rm
        OneCharString[1] = '\0';
        result = strtoul(OneCharString, NULL, 16);
        imemin_instructions_array[i].rm = (uint8_t)result;
        strncpy(ThreeCharString, buffer + 6, 3); //immediate 1
        ThreeCharString[3] = '\0';
        result = strtoul(ThreeCharString, NULL, 16);
        if ((ThreeCharString[0] == 'F') || ThreeCharString[0] == 'f'){
            result = result | neg_mask; //trouble with casting, cause 12 bit var gets into 16 bits var, deafult 0 extantion.
        };
        imemin_instructions_array[i].immediate1 = (int16_t)result;
        strncpy(ThreeCharString, buffer + 9, 3); //immediate 2
        ThreeCharString[3] = '\0';
        result = strtoul(ThreeCharString, NULL, 16);
        if ((ThreeCharString[0] == 'F') || ThreeCharString[0] == 'f'){
            result = result | neg_mask; //trouble with casting, cause 12 bit var gets into 16 bits var, deafult 0 extantion.
        };
        imemin_instructions_array[i].immediate2 = (int16_t)result;
        i += 1;
    }
    fclose(file_r);
    return 1;
};

//reads from dmemin.txt the data memory, and inits the "dmem_array"
int Init_dmem_array() {
    unsigned int i = 0;
    uint32_t line;
    FILE* file_r = fopen(dmemin_path, "r");
    char buffer[9]; //cause every line is 8 chars
    if (file_r == NULL) { fprintf(stderr, "could not open path %s in r mode\n", dmemin_path); return 0; };
    while (fgets(buffer, sizeof(buffer), file_r) != NULL) {
        if (buffer[0] != '\n'){
            line = (uint32_t)strtoul(buffer, NULL, 16);
            dmem_array[i] = line;
            i += 1;
        };
    };
    fclose(file_r);
    return 1;
};


//reads from diskin.txt.
int Load_diskin() {
    int i = 0, j = 0;
    char buffer[9]; // cause every line is 8 chars
    uint32_t line;
    FILE* file_r = fopen(diskin_path, "r");
    if (file_r == NULL) {
        fprintf(stderr, "could not open path %s in r mode\n", diskin_path);
        return 0;
    }
    while (fgets(buffer, sizeof(buffer), file_r) != NULL) {
        if (buffer[0] != '\n'){
            line = (uint32_t)strtoul(buffer, NULL, 16);
            disk_matrix[i][j] = line;
            j++;
            if (j == 128) {
                i++;
                j = 0;
            }
        };
    };
    fclose(file_r);
    return 1;
};

// Creates a new irq2 node and appends it to the irq2 list
void add_irq2_node(uint64_t number) {
    irq2_node* new_node = (irq2_node*)malloc(sizeof(irq2_node));
    if (new_node == NULL) { exit(1); }

    if (curr_irq2_node != NULL) { curr_irq2_node->next = new_node; }
    else { head_irq2_list = new_node; }
    curr_irq2_node = new_node;

    curr_irq2_node->cycle = number;
}

//reads from irg2in.txt, and inits the "irq2in_list". 
int Load_irq2in() {
    int index = 0;
    char chr, cur_num[200] = {'\0'};
    FILE* file_r = fopen(irq2in_path, "r");

    if (file_r == NULL) {
        fprintf(stderr, "could not open path %s in r mode\n", irq2in_path);
        return 0;
    }

    while ((chr = fgetc(file_r)) != EOF) {
        if (chr == '\n') {
            cur_num[index] = '\0';
            add_irq2_node(strtoul(cur_num, NULL, 10));
            index = 0;
        }
        else { cur_num[index++] = chr; }
    }

    fclose(file_r);
    curr_irq2_node = head_irq2_list;

    return 1;
}

//-----------Output functions----------//

// Helper Functions
FILE* open_in_mode(char* file_path, const char * file_name, const char * mode) {
    FILE* fd = fopen(file_path, mode);
    if (fd == NULL) { fprintf(stderr, "could not open file %s's path %s in %s mode\n", file_name, file_path, mode); };
    return fd;
}

FILE* open_w_then_a(char * file_path, const char * file_name) {
    FILE* file_w, * file_a;
    if ((file_w = open_in_mode(file_path, file_name, "w")) == NULL) { return NULL; }
    fclose(file_w);
    if ((file_a = open_in_mode(file_path, file_name, "a")) == NULL) { return NULL; }
    return file_a;
}

//take an instruction struct. return uint64_t that represents it.
uint64_t convert_instruction_to_bits(Instruction inst) {
    uint64_t ret_val = 0;
    uint64_t tmp_mask = 0;
    uint64_t mask_4bit = 15;
    uint64_t mask_8bit = 255;
    uint64_t mask_12bit = 4095;
    tmp_mask = mask_8bit & (uint64_t)inst.opcode;
    tmp_mask = tmp_mask << 40;
    ret_val = ret_val | tmp_mask;

    tmp_mask = mask_4bit & (uint64_t)inst.rd;
    tmp_mask = tmp_mask << 36;
    ret_val = ret_val | tmp_mask;

    tmp_mask = mask_4bit & (uint64_t)inst.rs;
    tmp_mask = tmp_mask << 32;
    ret_val = ret_val | tmp_mask;

    tmp_mask = mask_4bit & (uint64_t)inst.rt;
    tmp_mask = tmp_mask << 28;
    ret_val = ret_val | tmp_mask;

    tmp_mask = mask_4bit & (uint64_t)inst.rm;
    tmp_mask = tmp_mask << 24;
    ret_val = ret_val | tmp_mask;

    tmp_mask = mask_12bit & (uint64_t)inst.immediate1;
    tmp_mask = tmp_mask << 12;
    ret_val = ret_val | tmp_mask;

    tmp_mask = mask_12bit & (uint64_t)inst.immediate2;
    tmp_mask = tmp_mask;
    ret_val = ret_val | tmp_mask;
    return ret_val;
};

//finds the index that its only zeros after
int find_limit_index_in_dmem(){
    int i;
    int index = 0;
    for (i = 0; i < 4096; i++) {
        if (dmem_array[i] != 0) {
            index = i+1;
        };
    };
    return index;
};

//finds the index that its only zeros after
int find_limit_index_in_diskout(){
    int i;
    int j;
    int index = 0;
    for (i = 0; i < 128; i++) { 
        for (j = 0; j < 128; j++) { 
            if (disk_matrix[i][j] != 0){
                index = i*128 + j + 1;
            };
        };
    };
    return index;
};

//finds the index that its only zeros after
int find_limit_index_in_monitor(){
    int i;
    int j;
    int index = 0;
    for (i = 0; i < 256; i++) { 
        for (j = 0; j < 256; j++) { 
            if (monitor[i][j] != 0){
                index = i*256 + j + 1;
            };
        };
    };
    return index;
};

//creates dmemout.txt file, based on dmem_array
int Create_dmemout_txt() {
    int i;
    int index = find_limit_index_in_dmem();
    FILE* file_a = open_w_then_a(dmemout_path, "dmemout.txt");
    if (file_a == NULL) { return -1; }

    for (i = 0; i < index; i++) {
        fprintf(file_a, "%08" PRIX32 "\n", dmem_array[i]);
    };

    fclose(file_a);
    return 1;
};

//creates regout.txt file, based on registers values
int Create_regout_txt() {
    int i;
    FILE* file_a = open_w_then_a(regout_path, "regout.txt");
    if (file_a == NULL) { return -1; }

    for (i = 3; i < 16; i++) { fprintf(file_a, "%08"PRIX32"\n", *(reg_pointer_array[i])); }

    fclose(file_a);
    return 1;
};

//creates trace.txt, based on trace_line_list
int Create_trace_txt() {
    int i;
    Trace_line_node *tmp;
    FILE* file_a = open_w_then_a(trace_path, "trace.txt");
    if (file_a == NULL) { return -1; }
    curr_trace_line_node = head_trace_line_list;

    while (curr_trace_line_node != NULL) {
        fprintf(file_a, "%03"PRIX16" ", curr_trace_line_node->trace_line.pc);
        fprintf(file_a, "%012"PRIX64, convert_instruction_to_bits(curr_trace_line_node->trace_line.inst));
        for (i = 0; i < 16; i++) {
            fprintf(file_a, " %08x", curr_trace_line_node->trace_line.reg_pointer_array_snap[i]);
        };
        fprintf(file_a, "\n");
        tmp = curr_trace_line_node;
        curr_trace_line_node = curr_trace_line_node->next;
        free(tmp);
    };

    fclose(file_a);
    return 1;
};

//creates hwregtrace.txt, based on hw_trace_line_list.
int Create_hwregtrace_txt() {
    Hw_trace_line_node* tmp;
    FILE* file_a = open_w_then_a(hw_reg_trace_path, "hwregtrace.txt");
    if (file_a == NULL) { return -1; };
    curr_hw_trace_line_node = head_hw_trace_line_list;

    while (curr_hw_trace_line_node != NULL) {
        fprintf(file_a, "%llu ", curr_hw_trace_line_node->hw_trace_line.cycle);
        if (curr_hw_trace_line_node->hw_trace_line.read == 0) { fprintf(file_a, "WRITE "); }
        else { fprintf(file_a, "READ "); }
        fprintf(file_a, "%s ", IO_reg_names[curr_hw_trace_line_node->hw_trace_line.reg_num]);
        fprintf(file_a, "%08x\n", curr_hw_trace_line_node->hw_trace_line.data);
        tmp = curr_hw_trace_line_node;
        curr_hw_trace_line_node = curr_hw_trace_line_node->next;
        free(tmp);
    };
    fclose(file_a);
    return 1;
};

//creates cycles.txt, based on cycle_counter.
int Create_cycles_txt() {
    FILE* file = open_in_mode(cycles_path, "cycles.txt", "w");
    if (file == NULL) { return -1; }

    fprintf(file, "%llu\n", cycle_counter);
    fclose(file);
    return 1;
};

//creates leds.txt, based on leds_trace_list.
int Create_leds_txt() {
    FILE* file_a = open_w_then_a(leds_path, "leds.txt");
    if (file_a == NULL) { return -1; }
    curr_leds_trace_node = head_leds_trace_list;

    while (curr_leds_trace_node != NULL) {
        fprintf(file_a, "%llu ", curr_leds_trace_node->display_trace_line.cycle);
        fprintf(file_a, "%08x\n", curr_leds_trace_node->display_trace_line.display_snapshot);
        curr_leds_trace_node = curr_leds_trace_node->next;
    };

    fclose(file_a);
    return 1;
};

//creates display7seg.txt, based on dis7seg_list.
int Create_display7seg_txt() {
    FILE* file_a = open_w_then_a(display7seg_path, "display7seg.txt");
    if (file_a == NULL) { return -1; }
    curr_dis7seg_trace_node = head_dis7seg_trace_list;

    while (curr_dis7seg_trace_node != NULL) {
        fprintf(file_a, "%llu ", curr_dis7seg_trace_node->display_trace_line.cycle);
        fprintf(file_a, "%08x\n", curr_dis7seg_trace_node->display_trace_line.display_snapshot);
        curr_dis7seg_trace_node = curr_dis7seg_trace_node->next;
    };

    fclose(file_a);
    return 1;
};

//creates diskout.txt
int Create_diskout_txt() {
    int i, j;
    int index = find_limit_index_in_diskout();
    FILE* file_a = open_w_then_a(diskout_path, "diskout.txt");
    if (file_a == NULL) { return -1; }

    for (i = 0; i < ((index-(index%128))/128); i++) { for (j = 0; j < 128; j++) { fprintf(file_a, "%08"PRIX8"\n", disk_matrix[i][j]); } }
    for (j=0; j < index%128; j++){
        fprintf(file_a, "%08"PRIX8"\n", disk_matrix[i][j]); 
    };
    fclose(file_a);
    return 1;
}

//creates monitor.txt, based on monitor[][].
int Create_monitor_txt() {
    int i, j;
    int index = find_limit_index_in_monitor();
    FILE* file_a = open_w_then_a(monitor_txt_path, "monitor.txt");
    if (file_a == NULL) { return -1; }

    for (i = 0; i < ((index - (index % 256))/256); i++) { for (j = 0; j < 256; j++) { fprintf(file_a, "%02"PRIX8"\n", monitor[i][j]); } }
    for (j=0; j < (index % 256); j++){
        fprintf(file_a, "%02"PRIX8"\n", monitor[i][j]); 
    };

    fclose(file_a);
    return 1;
};

//------------IO Command Helpers------------//

// Checks if out command wrote something to a cmd IO register and executes accordingly
void handle_cmds(uint32_t io_reg_ind) {
    int i;

    if (io_reg_ind == 14) { // Disk
        if (diskcmd == 1) { for (i = 0; i < 128; i++) { dmem_array[(i + ((int)diskbuffer)) % 4096] = disk_matrix[disksector][i]; } }
        else if (diskcmd == 2) { for (i = 0; i < 128; i++) { disk_matrix[disksector][i] = dmem_array[(i + ((int)diskbuffer)) % 4096]; } }
        else { return; }
        DISK_TIMEOUT = (uint64_t)(cycle_counter + 1024);
        diskstatus = 1;
    }
    // Monitor
    else if (io_reg_ind == 22 && monitorcmd == 1) { monitor[((int)monitoraddr) / 256][((int)monitoraddr) % 256] = monitordata; }
}

// Creates a new hw trace node and appends it to the hw trace list
void add_hw_trace_node(int reg_num, int read) {
    Hw_trace_line_node* new_node = (Hw_trace_line_node*)malloc(sizeof(Hw_trace_line_node));
    if (new_node == NULL) { exit(1); }

    if (curr_hw_trace_line_node != NULL) { curr_hw_trace_line_node->next = new_node; }
    else { head_hw_trace_line_list = new_node; }
    curr_hw_trace_line_node = new_node;

    curr_hw_trace_line_node->next = NULL;
    curr_hw_trace_line_node->hw_trace_line.cycle = cycle_counter;
    curr_hw_trace_line_node->hw_trace_line.read = (uint8_t) read;
    curr_hw_trace_line_node->hw_trace_line.reg_num = (uint8_t) reg_num;
    curr_hw_trace_line_node->hw_trace_line.data = *IO_reg_pointer_array[reg_num];
}

// Creates a new leds trace node and appends it to the leds trace list
void add_leds_trace_node() {
    Display_trace_line_node* new_node = (Display_trace_line_node*) malloc(sizeof(Display_trace_line_node));
    if (new_node == NULL) { exit(1); }

    if (curr_leds_trace_node != NULL) { curr_leds_trace_node->next = new_node; }
    else { head_leds_trace_list = new_node; }
    curr_leds_trace_node = new_node;

    curr_leds_trace_node->next = NULL;
    curr_leds_trace_node->display_trace_line.cycle = cycle_counter;
    curr_leds_trace_node->display_trace_line.display_snapshot = leds;
}

// Creates a new 7 segment display trace node and appends it to the 7 segment display trace list
void add_dis7seg_trace_node() {
    Display_trace_line_node* new_node = (Display_trace_line_node*)malloc(sizeof(irq2_node));
    if (new_node == NULL) { exit(1); }

    if (curr_dis7seg_trace_node != NULL) { curr_dis7seg_trace_node->next = new_node; }
    else { head_dis7seg_trace_list = new_node; }
    curr_dis7seg_trace_node = new_node;

    curr_dis7seg_trace_node->next = NULL;
    curr_dis7seg_trace_node->display_trace_line.cycle = cycle_counter;
    curr_dis7seg_trace_node->display_trace_line.display_snapshot = display7seg;
}

// Func handles tracing for hardware commands
void io_command_trace(int reg_num, int read) {
    add_hw_trace_node(reg_num, read);

    if (!read) {
        if (reg_num == 9) { add_leds_trace_node(); }
        else if (reg_num == 10) { add_dis7seg_trace_node(); }
    }
}

//-----------Commands functions----------//

//gets register's indexes, performs the add command as it describes.
void do_add_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) { return; }; //read-only registers.
    int32_t val = *reg_pointer_array[rs_i] + *reg_pointer_array[rt_i] + *reg_pointer_array[rm_i];
    *reg_pointer_array[rd_i] = val;
};

//gets register's indexes, performs the sub command as it describes.
void do_sub_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) { return; }; //read-only registers.
    int32_t val = *reg_pointer_array[rs_i] - *reg_pointer_array[rt_i] - *reg_pointer_array[rm_i];
    *reg_pointer_array[rd_i] = val;
};

//gets register's indexes, performs the mac command as it describes.
void do_mac_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) { return; }; //read-only registers.
    int32_t val = *reg_pointer_array[rs_i] * *reg_pointer_array[rt_i] + *reg_pointer_array[rm_i];
    *reg_pointer_array[rd_i] = val;
};

//gets register's indexes, performs the and command as it describes.
void do_and_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) { return; }; //read-only registers.
    int32_t val = *reg_pointer_array[rs_i] & *reg_pointer_array[rt_i] & *reg_pointer_array[rm_i];
    *reg_pointer_array[rd_i] = val;
};

//gets register's indexes, performs the or command as it describes.
void do_or_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) { return; }; //read-only registers.
    int32_t val = *reg_pointer_array[rs_i] | *reg_pointer_array[rt_i] | *reg_pointer_array[rm_i];
    *reg_pointer_array[rd_i] = val;
};

//gets register's indexes, performs the xor command as it describes.
void do_xor_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) { return; }; //read-only registers.
    int32_t val = *reg_pointer_array[rs_i] ^ *reg_pointer_array[rt_i] ^ *reg_pointer_array[rm_i];
    *reg_pointer_array[rd_i] = val;
};

//gets register's indexes, performs the sll command as it describes.
void do_sll_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i) {
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) { return; }; //read-only registers.
    int32_t val = *reg_pointer_array[rs_i] << *reg_pointer_array[rt_i];
    *reg_pointer_array[rd_i] = val;
};

//gets register's indexes, performs the sra command as it describes.
void do_sra_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i) {
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) { return; }; //read-only registers.
    int32_t val = *reg_pointer_array[rs_i] >> *reg_pointer_array[rt_i];
    int32_t mask = 0xFFFFFFFF; //All 1's
    mask = mask << (32 - (*reg_pointer_array[rt_i]));
    val = val | mask;
    *reg_pointer_array[rd_i] = val;
}

//gets register's indexes, performs the srl command as it describes.
void do_srl_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i) {
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) { return; }; //read-only registers.
    int32_t val = *reg_pointer_array[rs_i] >> *reg_pointer_array[rt_i];
    *reg_pointer_array[rd_i] = val;
};

//gets register's indexes, performs the beq command as it describes.
void do_beq_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    uint16_t mask = 0xFFF; //00000111111111111
    if (*reg_pointer_array[rs_i] == *reg_pointer_array[rt_i]) {
        PC = (uint16_t) ((*reg_pointer_array[rm_i] & mask) - 1); //the casting takes the 16 lower bits.
    };
};

//gets register's indexes, performs the bne command as it describes.
void do_bne_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    uint16_t mask = 0xFFF; //0000111111111111
    if (*reg_pointer_array[rs_i] != *reg_pointer_array[rt_i]) {
        PC = (uint16_t)((*reg_pointer_array[rm_i] & mask) - 1); //the casting takes the 16 lower bits.
    };
};

//gets register's indexes, performs the blt command as it describes.
void do_blt_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    uint16_t mask = 0xFFF; //0000111111111111
    if (*reg_pointer_array[rs_i] < *reg_pointer_array[rt_i]) {
        PC = (uint16_t)((*reg_pointer_array[rm_i] & mask) - 1); //the casting takes the 16 lower bits.
    };
};

//gets register's indexes, performs the bgt command as it describes.
void do_bgt_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    uint16_t mask = 0xFFF; //0000111111111111
    if (*reg_pointer_array[rs_i] > *reg_pointer_array[rt_i]) {
        PC = (uint16_t)((*reg_pointer_array[rm_i] & mask) - 1); //the casting takes the 16 lower bits.
    };
};

//gets register's indexes, performs the ble command as it describes.
void do_ble_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    uint16_t mask = 0xFFF; //0000111111111111
    if (*reg_pointer_array[rs_i] <= *reg_pointer_array[rt_i]) {
        PC = (uint16_t)((*reg_pointer_array[rm_i] & mask) - 1); //the casting takes the 16 lower bits.
    };
};

//gets register's indexes, performs the bge command as it describes.
void do_bge_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    uint16_t mask = 0xFFF; //0000111111111111
    if (*reg_pointer_array[rs_i] >= *reg_pointer_array[rt_i]) {
        PC = (uint16_t)((*reg_pointer_array[rm_i] & mask) - 1); //the casting takes the 16 lower bits.
    };
};

//gets register's indexes, performs the jal command as it describes.
void do_jal_command(uint8_t rd_i, uint8_t rm_i) {
    if (rd_i != 0 && rd_i != 1 && rd_i != 2) { 
        *reg_pointer_array[rd_i] = PC + 1;
    }; //read-only registers.
    uint16_t mask = 0xFFF; //0000111111111111
    PC = (int16_t)((*reg_pointer_array[rm_i] & mask) -1); //the casting takes the 16 lower bits.
};

//gets register's indexes, performs the lw command as it describes.
void do_lw_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) { return; }; //read-only registers.
    int mem_index = (*reg_pointer_array[rs_i] + *reg_pointer_array[rt_i]) % 4096;
    *reg_pointer_array[rd_i] = dmem_array[mem_index] + *reg_pointer_array[rm_i];
};

//gets register's indexes, performs the sw command as it describes.
void do_sw_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    int mem_index = (*reg_pointer_array[rs_i] + *reg_pointer_array[rt_i]) % 4096;
    uint32_t val = *reg_pointer_array[rm_i] + *reg_pointer_array[rd_i];
    dmem_array[mem_index] = val;
};

//gets register's indexes, performs the reti command as it describes.
void do_reti_command() {
    PC = *IO_reg_pointer_array[7] - 1;

    if (CURR_SIG == 0) { irq0status = 0; }
    else if (CURR_SIG == 1) { irq1status = 0; }
    else if (CURR_SIG == 2) { irq2status = 0; }
    CURR_SIG = -1;
};

//gets register's indexes, performs the in command as it describes.
int do_in_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i) {
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) { return 1; }; //read-only registers.
    int index = (int)(*reg_pointer_array[rs_i] + *reg_pointer_array[rt_i]);
    if (index > 22) {
        perror("Error at in command. R[rs]+R[rt] is to big for an IORegister index.");
        return -1;
    };
    *reg_pointer_array[rd_i] = *IO_reg_pointer_array[index];
    io_command_trace(index, 1);
    return 1;
};

//int cnt2 = 0;
//gets register's indexes, performs the out command as it describes.
int do_out_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i) {
    int index = (int)((int32_t)*reg_pointer_array[rs_i] + (int32_t)*reg_pointer_array[rt_i]);
    //if (cnt2 < 5){
    //    printf("in register rs is: %d\n", *reg_pointer_array[rs_i]);
    //    printf("in register rt is: %d\n", *reg_pointer_array[rt_i]);
    //    printf("out command index is: %d\n", index);
    //};
    //cnt2 += 1;
    if (index > 22) {
        perror("Error at out command. R[rs]+R[rt] is to big for an IORegister index.");
        return -1;
    };
    *IO_reg_pointer_array[index] = *reg_pointer_array[rm_i];
    handle_cmds(index);
    io_command_trace(index, 0);
    return 1;
};

//gets the instruction struct. calls the right function to commit it, with the right parameters.
//returns -1 in error, 1 in success and 0 if the command is halt.
int commit_the_instruction(Instruction inst) {
    switch (inst.opcode) {
    case 0:
        do_add_command(inst.rd, inst.rs, inst.rt, inst.rm);
        return 1;
    case 1:
        do_sub_command(inst.rd, inst.rs, inst.rt, inst.rm);
        return 1;
    case 2:
        do_mac_command(inst.rd, inst.rs, inst.rt, inst.rm);
        return 1;
    case 3:
        do_and_command(inst.rd, inst.rs, inst.rt, inst.rm);
        return 1;
    case 4:
        do_or_command(inst.rd, inst.rs, inst.rt, inst.rm);
        return 1;
    case 5:
        do_xor_command(inst.rd, inst.rs, inst.rt, inst.rm);
        return 1;
    case 6:
        do_sll_command(inst.rd, inst.rs, inst.rt);
        return 1;
    case 7:
        do_sra_command(inst.rd, inst.rs, inst.rt);
        return 1;
    case 8:
        do_srl_command(inst.rd, inst.rs, inst.rt);
        return 1;
    case 9:
        do_beq_command(inst.rs, inst.rt, inst.rm);
        return 1;
    case 10:
        do_bne_command(inst.rs, inst.rt, inst.rm);
        return 1;
    case 11:
        do_blt_command(inst.rs, inst.rt, inst.rm);
        return 1;
    case 12:
        do_bgt_command(inst.rs, inst.rt, inst.rm);
        return 1;
    case 13:
        do_ble_command(inst.rs, inst.rt, inst.rm);
        return 1;
    case 14:
        do_bge_command(inst.rs, inst.rt, inst.rm);
        return 1;
    case 15:
        do_jal_command(inst.rd, inst.rm);
        return 1;
    case 16:
        do_lw_command(inst.rd, inst.rs, inst.rt, inst.rm);
        return 1;
    case 17:
        do_sw_command(inst.rd, inst.rs, inst.rt, inst.rm);
        return 1;
    case 18:
        do_reti_command();
        return 1;
    case 19:
        return do_in_command(inst.rd, inst.rs, inst.rt);
    case 20:
        return do_out_command(inst.rs, inst.rt, inst.rm);
    case 21:
        return 0;
    default:
        perror("Opcode is not recognize (not in 0-21). error in commit_the_instruction\n");
        return -1;
    };
};

// ----------------------- Main and Main Helpers -------------------------------------- //

// Sets up the file_name parameters
void set_up_files(char* paths[]) {
    imemin_path = paths[0];
    dmemin_path = paths[1];
    diskin_path = paths[2];
    irq2in_path = paths[3];
    dmemout_path = paths[4];
    regout_path = paths[5];
    trace_path = paths[6];
    hw_reg_trace_path = paths[7];
    cycles_path = paths[8];
    leds_path = paths[9];
    display7seg_path = paths[10];
    diskout_path = paths[11];
    monitor_txt_path = paths[12];
    monitor_yuv_path = paths[13];
}

// Loads needed files into their presperctive constants
int load_files() {
    if (Load_diskin() && Load_irq2in() && Init_dmem_array() && Init_instructions_array()) { return 1; }
    exit(1);
}

// Creates all needed out files
void create_out_files() {
    Create_display7seg_txt();
    Create_dmemout_txt();
    Create_cycles_txt();
    Create_leds_txt();
    Create_trace_txt();
    Create_hwregtrace_txt();
    Create_regout_txt();
    Create_diskout_txt();
    Create_monitor_txt();
}

// Copy regs array
void copy_regs_array(int32_t* array) {
    int i;
    for (i = 0; i < 16; i++) { array[i] = *reg_pointer_array[i]; }
}

// Creates a new trace node and appends it to the trace lists
void add_trace_node() {
    Trace_line_node *new_node = (Trace_line_node *) malloc(sizeof(Trace_line_node));
    if (new_node == NULL) { exit(1); }
    
    if (curr_trace_line_node != NULL) { curr_trace_line_node->next = new_node; }
    else { head_trace_line_list = new_node; }
    curr_trace_line_node = new_node;

    curr_trace_line_node->next = NULL;
    copy_regs_array(curr_trace_line_node->trace_line.reg_pointer_array_snap);
    curr_trace_line_node->trace_line.pc = PC; 
    curr_trace_line_node->trace_line.inst = imemin_instructions_array[PC];
}

// Handles raising of interrupt status in case any of the interupt cases accured
void handle_ints() {
    int cnd0 = irq0enable && irq0status, cnd1 = irq1enable && irq1status, cnd2 = irq2enable && irq2status;

    // Raise flags
    if (CURR_SIG == -1 && (cnd0 || cnd1 || cnd2)) {
        if (cnd0) { CURR_SIG = 0; }
        else if (cnd1) { CURR_SIG = 1; }
        else if (cnd2) { CURR_SIG = 2; }
        irqreturn = PC;
        PC = irqhandler;
    }

    // Timer
    if (timerenable) {
        if (timercurrent == timermax) {
            irq0status = 1;
            timercurrent = 0;
        }
        else { timercurrent++; }
    }

    // Disk
    if (DISK_TIMEOUT != 0 && cycle_counter == DISK_TIMEOUT) {
        irq1status = 1;
        diskstatus = 0;
        DISK_TIMEOUT = 0;
    }

    // irq2
    if (curr_irq2_node != NULL && cycle_counter == curr_irq2_node->cycle) {
        irq2status = 1;
        curr_irq2_node = curr_irq2_node->next;
    }
    else { irq2status = 0; }
}

// Reads and executes am instruction
int exec_instruction() {
    int halt;
    uint8_t opcode = imemin_instructions_array[PC].opcode;

    if ((halt = commit_the_instruction(imemin_instructions_array[PC])) == -1) { exit(-1); }

    cycle_counter++;
    clks++;
    PC++;

    return halt;
}

// Updates immediate 1 & 2's values before executing a command
void update_immediates(){
    R_imm1 = (int32_t) imemin_instructions_array[PC].immediate1;
    R_imm2 = (int32_t) imemin_instructions_array[PC].immediate2;
};

int main(int argc, char* argv[]) {
    // Set Up
    set_up_files(argv + 1);
    load_files();
    
    // Main
    do {
        handle_ints();
        update_immediates();
        add_trace_node();
    } while (exec_instruction());

    // Tear Down
    create_out_files();
}
