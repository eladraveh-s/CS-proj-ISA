#include <stdint.h>
#include <stdio.h>
#include <string.h>

///The simulator part of the ISA Project

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
    uint32_t reg_pointer_array_snap[16];
} Trace_line;

typedef struct s_trace_line_node {
    struct s_trace_line_node * next;                                                                   
    Trace_line trace_line;
} Trace_line_node;

typedef struct s_hw_trace_line {
    uint64_t cycle;
    uint8_t read; //0 is write, 1 is read.
    uint8_t reg_num; //as described in I/O reg table.
    uint32_t data;
} Hw_trace_line;

typedef struct s_hw_trace_line_node {
    struct s_hw_trace_line_node * next;                                                                   
    Hw_trace_line hw_trace_line;
} Hw_trace_line_node;

typedef struct s_display_trace_line {
    uint64_t cycle;
    uint32_t display_snapshot;
} Display_trace_line;

typedef struct s_display_trace_line_node {
    struct s_display_trace_line * next;                                                                   
    Display_trace_line display_trace_line;
} Display_trace_line_node;

//////////////////////////////////////////
//-----------Global Varivables----------//
//////////////////////////////////////////
uint16_t PC;
//------Registers------//
uint32_t  R_Zero;
uint32_t  R_imm1;
uint32_t  R_imm2;
uint32_t  R_v0;
uint32_t  R_a0;
uint32_t  R_a1;
uint32_t  R_a2;
uint32_t  R_t0;
uint32_t  R_t1;
uint32_t  R_t2;
uint32_t  R_s0;
uint32_t  R_s1;
uint32_t  R_s2;
uint32_t  R_gp;
uint32_t  R_sp;
uint32_t  R_ra;

uint32_t* reg_pointer_array[] = {
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
uint8_t irq0enable;
uint8_t irq1enable;
uint8_t irq2enable;
uint8_t irq0status;
uint8_t irq1status;
uint8_t irq2status;
uint16_t irqhandler;
uint16_t irqreturn;
uint32_t clks;
uint32_t leds;
uint32_t display7seg;
uint8_t timerenable;
uint32_t timercurrent;
uint32_t timermax;
uint8_t diskcmd;
uint8_t disksector;
uint16_t diskbuffer;
uint8_t diskstatus;
uint16_t monitoraddr;
uint8_t monitordata;
uint8_t monitorcmd;

uint32_t* IO_reg_pointer_array[] = {
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
    &monitoraddr,
    &monitordata,
    &monitorcmd
};

//------Monitor------//
uint8_t monitor[256][256];

Instruction imemin_instructions_array[4096];
uint32_t dmem_array[4096];
Trace_line_node* head_trace_line_list;
Trace_line_node* curr_trace_line_node;
Hw_trace_line_node* head_hw_trace_line_list;
Hw_trace_line_node* curr_hw_trace_line_node;
Display_trace_line_node* head_leds_trace_list;
Display_trace_line_node* curr_leds_trace_list;
Display_trace_line_node* head_dis7seg_trace_list;
Display_trace_line_node* curr_dis7seg_trace_list;
Display_trace_line_node* head_irq2in_list;
Display_trace_line_node* curr_irg2in_list;

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
char *imemin_path;
char *dmemin_path;
char *diskin_path;
char *irq2in_path;
char *dmemout_path;
char *regout_path;
char *trace_path;
char *hw_reg_trace_path;
char *cycles_path;
char *leds_path;
char *display7seg_path;
char *diskout_path;
char *monitor_txt_path;
char *monitor_yuv_path;

//////////////////////////////////////////
//-----------Simulator Functions--------//
//////////////////////////////////////////

//-----------Init functions-------------//
//reads from imemin.txt the instructions, and inits the "imemin_instructions_array" 
int Init_instructions_array(){
    unsigned int i = 0;
    Instruction inst;
    char ThreeCharString[4];  // Including the null terminator
    char TwoCharString[3];  // Including the null terminator
    char OneCharString[2];  // Including the null terminator
    unsigned int result;
    FILE * file_r = fopen(imemin_path, "r");
    char buffer[13]; //cause every line is 12 chars
    if (file_r == NULL){fprintf(stderr, "could not open path %s in r mode\n", imemin_path); return -1;};
    while (fgets(buffer, sizeof(buffer), file_r) != NULL) {
        strncpy(TwoCharString, buffer, 1); //opcode
        TwoCharString[2] = '\0'; 
        result = strtoul(TwoCharString, NULL, 16);
        imemin_instructions_array[i].opcode = (uint8_t)result;
        strncpy(OneCharString, buffer + 2, 2); //rd
        OneCharString[1] = '\0'; 
        result = strtoul(OneCharString, NULL, 16);
        imemin_instructions_array[i].rd = (uint8_t)result;
        strncpy(OneCharString, buffer + 3, 3); //rs
        OneCharString[1] = '\0'; 
        result = strtoul(OneCharString, NULL, 16);
        imemin_instructions_array[i].rs = (uint8_t)result;
        strncpy(OneCharString, buffer + 4, 4); //rt
        OneCharString[1] = '\0'; 
        result = strtoul(OneCharString, NULL, 16);
        imemin_instructions_array[i].rt = (uint8_t)result;
        strncpy(OneCharString, buffer + 5, 5); //rm
        OneCharString[1] = '\0'; 
        result = strtoul(OneCharString, NULL, 16);
        imemin_instructions_array[i].rm = (uint8_t)result;
        strncpy(ThreeCharString, buffer + 6, 8); //immediate 1
        ThreeCharString[4] = '\0'; 
        result = strtoul(ThreeCharString, NULL, 16);
        imemin_instructions_array[i].immediate1 = (int16_t)result;
        strncpy(ThreeCharString, buffer + 9, 11); //immediate 2
        ThreeCharString[4] = '\0'; 
        result = strtoul(ThreeCharString, NULL, 16);
        imemin_instructions_array[i].immediate2 = (int16_t)result;
        i += 1;
    }
    fclose(file_r);
    return 1;
};

//reads from dmemin.txt the data memory, and inits the "dmem_array"
int Init_dmem_array(){
    unsigned int i = 0;
    uint32_t line;
    FILE * file_r = fopen(dmemin_path, "r");
    char buffer[13]; //cause every line is 12 chars
    if (file_r == NULL){fprintf(stderr, "could not open path %s in r mode\n", dmemin_path); return -1;};
    while (fgets(buffer, sizeof(buffer), file_r) != NULL) {
        line = (uint32_t)strtoul(buffer, NULL, 16);
        dmem_array[i] = line;
        i+= 1;
    };
    fclose(file_r);
    return 1;
};

//reads from diskin.txt.
void Load_diskin();

//reads from irg2in.txt, and inits the "irq2in_list". 
int Load_irq2in(){
    uint64_t cycle;
    FILE * file_r = fopen(irq2in_path, "r");
    Display_trace_line_node * cur_node;
    Display_trace_line_node * prev_node;
    char buffer[256]; //change? is it big enough?
    if (file_r == NULL){fprintf(stderr, "could not open path %s in r mode\n", irq2in_path); return -1;};
    while (fgets(buffer, sizeof(buffer), file_r) != NULL) {
        cur_node = (Display_trace_line_node*)malloc(sizeof(Display_trace_line_node));
        if (prev_node != NULL){
            prev_node->next = cur_node;
        }
        else {
            head_irq2in_list = cur_node;
        };
        cycle = (unsigned int)strtoul(buffer, NULL, 10);
        cur_node->display_trace_line.cycle = cycle;
        prev_node = cur_node;
    };
    fclose(file_r);
    return 1;
};

//-----------Output functions----------//

// Helper Functions
FILE * open_in_mode(char *file_path, char *file_name, char *mode) {
    FILE* fd = fopen(file_path, mode);
    if (fd == NULL) {fprintf(stderr, "could not open file %s's path %s in %s mode\n", file_name, file_path, mode);};
    return fd;
}

FILE * open_w_then_a(char *file_path, char *file_name) {
    FILE *file_w, *file_a;
    if ((file_w = open_in_mode(file_path, file_name, "w")) == NULL) {return NULL;}
    fclose(file_w);
    if ((file_a = open_in_mode(file_path, file_name, "a")) == NULL) {return NULL;}
    return file_a;
}

//creates dmemout.txt file, based on dmem_array
int Create_dmemout_txt(){
    FILE* file_a = open_w_then_a(dmemout_path, "dmemout.txt");
    if (file_a == NULL) {return -1;}

    for (int i; i < 4096; i++) {
        fprintf(file_a, "%08x\n", dmem_array[i]);
    };
    
    fclose(file_a);
    return 1;
};

//creates regout.txt file, based on registers values
int Create_regout_txt(){
    FILE *file_a = open_w_then_a(regout_path, "regout.txt");
    if (file_a == NULL) {return -1;}

    for (int i; i < 16; i++){
        fprintf(file_a, "%08x\n", *(reg_pointer_array[i]));
    };

    fclose(file_a);
    return 1;
};

//creates trace.txt, based on trace_line_list
int Create_trace_txt(){
    FILE* file_a = open_w_then_a(trace_path, "trace.txt");
    Trace_line_node * cur_trace_node = head_trace_line_list;

    if (file_a == NULL) {return -1;}

    while (cur_trace_node != NULL){
        fprintf(file_a, "%03x ", cur_trace_node->trace_line.pc);
        fprintf(file_a, "%12x", convert_instruction_to_bits(cur_trace_node->trace_line.inst));
        for(int i; i < 16; i++){
           fprintf(file_a, " %08x", cur_trace_node->trace_line.reg_pointer_array_snap[i]); 
        };
        fprintf(file_a, "\n");
        cur_trace_node = cur_trace_node->next;
    };

    fclose(file_a);
    return 1;  
};

//creates hwregtrace.txt, based on hw_trace_line_list.
int Create_hwregtrace_txt(){
    FILE *file_a = open_w_then_a(hw_reg_trace_path, "hwregtrace.txt");
    Hw_trace_line_node * cur_hwreg_node = head_hw_trace_line_list;
    if (file_a == NULL) {return -1;};
    while (cur_hwreg_node != NULL){
        fprintf(file_a, "%d ", cur_hwreg_node->hw_trace_line.cycle);
        if (cur_hwreg_node->hw_trace_line.read == 0) {
            fprintf(file_a, "WRITE ");
        }
        else {
            fprintf(file_a, "READ ");
        };
        fprintf(file_a, "%s ", IO_reg_names[cur_hwreg_node->hw_trace_line.reg_num]);
        fprintf(file_a, "%08x\n", cur_hwreg_node->hw_trace_line.data);
        cur_hwreg_node = cur_hwreg_node->next;
    };
    fclose(file_a);
    return 1;
};

//creates cycles.txt, based on cycle_counter.
int Create_cycles_txt(){
    FILE* file = open_in_mode(cycles_path, "cycles.txt", "w");
    if (file == NULL) {return -1;}

    fprintf(file, "%d\n", cycle_counter);
    fclose(file);
    return 1;
};

//creates leds.txt, based on leds_trace_list.
int Create_leds_txt(){
    FILE *file_a = open_w_then_a(leds_path, "leds.txt");
    Display_trace_line_node * cur_led_node = head_leds_trace_list;

    if (file_a == NULL) {return -1;}

    while (cur_led_node != NULL){
        fprintf(file_a, "%d ", cur_led_node->display_trace_line.cycle);
        fprintf(file_a, "%08x\n", cur_led_node->display_trace_line.display_snapshot);
        cur_led_node = cur_led_node->next;
    };

    fclose(file_a);
    return 1;  
};
//creates display7seg.txt, based on dis7seg_list.
void Create_display7seg_txt(){
    FILE* file_a = opan_w_then_a(display7seg_path, "display7seg.txt");
    Display_trace_line_node * cur_dis7seg_node = head_dis7seg_trace_list;

    if (file_a == NULL) {return -1;}

    while (cur_dis7seg_node != NULL){
        fprintf(file_a, "%d ", cur_dis7seg_node->display_trace_line.cycle);
        fprintf(file_a, "%08x\n", cur_dis7seg_node->display_trace_line.display_snapshot);
        cur_dis7seg_node = cur_dis7seg_node->next;
    };

    fclose(file_a);
    return 1;  
};

//creates diskout.txt
void Create_diskout_txt() {
    
};

//creates monitor.txt, based on monitor[][].
int Create_monitor_txt(){
    FILE* file_a = open_w_then_a(monitor_txt_path, "monitor.txt");
    if (file_a == NULL) {return -1;}

    for (int i; i < 256; i++){
        for (int j; j < 256; j++) {
            fprintf(file_a, "%02x\n", monitor[i][j]);
        };
    };
    
    fclose(file_a);
    return 1;
};

//take an instruction struct. return uint64_t that represents it.
uint64_t convert_instruction_to_bits(Instruction inst){
    uint64_t ret_val = 0;
    uint64_t tmp_mask = 0;
    tmp_mask = (uint64_t)inst.opcode << 40;
    ret_val = ret_val | tmp_mask;
    tmp_mask = (uint64_t)inst.rd << 36;
    ret_val = ret_val | tmp_mask;
    tmp_mask = (uint64_t)inst.rs << 32;
    ret_val = ret_val | tmp_mask;
    tmp_mask = (uint64_t)inst.rt << 28;
    ret_val = ret_val | tmp_mask;   
    tmp_mask = (uint64_t)inst.rm << 24;
    ret_val = ret_val | tmp_mask;
    tmp_mask = (uint64_t)inst.immediate1 << 12;
    ret_val = ret_val | tmp_mask;
    tmp_mask = (uint64_t)inst.immediate2;
    ret_val = ret_val | tmp_mask;
    return ret_val;
};

//Sets up the file_name parameters
void set_up_files(char *paths[]) {
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

//-----------Commands functions----------//

//gets register's indexes, performs the add command as it describes.
void do_add_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) {return;}; //read-only registers.
    int32_t val = (int32_t)*reg_pointer_array[rs_i] + (int32_t)*reg_pointer_array[rt_i] + (int32_t)*reg_pointer_array[rm_i];
    reg_pointer_array[rd_i] = (uint32_t)&val;
};

//gets register's indexes, performs the sub command as it describes.
void do_sub_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) {return;}; //read-only registers.
    int32_t val = (int32_t)*reg_pointer_array[rs_i] - (int32_t)*reg_pointer_array[rt_i] - (int32_t)*reg_pointer_array[rm_i];
    reg_pointer_array[rd_i] = (uint32_t)&val;
};

//gets register's indexes, performs the mac command as it describes.
void do_mac_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) {return;}; //read-only registers.
    int32_t val = (((int32_t)*reg_pointer_array[rs_i])*((int32_t)*reg_pointer_array[rt_i])) + (int32_t)*reg_pointer_array[rm_i];
    reg_pointer_array[rd_i] = (uint32_t)&val;
};


//gets register's indexes, performs the and command as it describes.
void do_and_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) {return;}; //read-only registers.
    uint32_t val = *reg_pointer_array[rs_i] & *reg_pointer_array[rt_i] & *reg_pointer_array[rm_i];
    reg_pointer_array[rd_i] = &val;
};

//gets register's indexes, performs the or command as it describes.
void do_or_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) {return;}; //read-only registers.
    uint32_t val = *reg_pointer_array[rs_i] | *reg_pointer_array[rt_i] | *reg_pointer_array[rm_i];
    reg_pointer_array[rd_i] = &val;
};

//gets register's indexes, performs the xor command as it describes.
void do_xor_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) {return;}; //read-only registers.
    uint32_t val = *reg_pointer_array[rs_i] ^ *reg_pointer_array[rt_i] ^ *reg_pointer_array[rm_i];
    reg_pointer_array[rd_i] = &val;
};


//gets register's indexes, performs the sll command as it describes.
void do_sll_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i){
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) {return;}; //read-only registers.
    uint32_t val = *reg_pointer_array[rs_i] << *reg_pointer_array[rt_i];
    reg_pointer_array[rd_i] = &val;
};

//gets register's indexes, performs the sra command as it describes.
void do_sra_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i){
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) {return;}; //read-only registers.
    uint32_t val = (int32_t)*reg_pointer_array[rs_i] >> (int32_t)*reg_pointer_array[rt_i];
    uint32_t mask = 4294967295; //the largest number, only 1's.
    mask << 32-(*reg_pointer_array[rt_i]);
    val = val | mask;
    reg_pointer_array[rd_i] = &val;
};

//gets register's indexes, performs the srl command as it describes.
void do_srl_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i){
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) {return;}; //read-only registers.
    uint32_t val = (uint32_t)*reg_pointer_array[rs_i] >> (uint32_t)*reg_pointer_array[rt_i];
    reg_pointer_array[rd_i] = &val;
};

//gets register's indexes, performs the beq command as it describes.
void do_beq_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    uint16_t mask = 4095; //0000111111111111
    if (*reg_pointer_array[rs_i] == *reg_pointer_array[rt_i]){
        PC = (uint16_t)*reg_pointer_array[rm_i] & mask; //the casting takes the 16 lower bits.
    };
    
};

//gets register's indexes, performs the bne command as it describes.
void do_bne_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    uint16_t mask = 4095; //0000111111111111
    if (*reg_pointer_array[rs_i] != *reg_pointer_array[rt_i]){
        PC = (uint16_t)*reg_pointer_array[rm_i] & mask; //the casting takes the 16 lower bits.
    };
    
};

//gets register's indexes, performs the blt command as it describes.
void do_blt_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    uint16_t mask = 4095; //0000111111111111
    if (*reg_pointer_array[rs_i] < *reg_pointer_array[rt_i]){
        PC = (uint16_t)*reg_pointer_array[rm_i] & mask; //the casting takes the 16 lower bits.
    }; 
};

//gets register's indexes, performs the bgt command as it describes.
void do_bgt_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    uint16_t mask = 4095; //0000111111111111
    if (*reg_pointer_array[rs_i] > *reg_pointer_array[rt_i]){
        PC = (uint16_t)*reg_pointer_array[rm_i] & mask; //the casting takes the 16 lower bits.
    }; 
};

//gets register's indexes, performs the ble command as it describes.
void do_ble_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    uint16_t mask = 4095; //0000111111111111
    if (*reg_pointer_array[rs_i] <= *reg_pointer_array[rt_i]){
        PC = (uint16_t)*reg_pointer_array[rm_i] & mask; //the casting takes the 16 lower bits.
    }; 
};

//gets register's indexes, performs the bge command as it describes.
void do_bge_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    uint16_t mask = 4095; //0000111111111111
    if (*reg_pointer_array[rs_i] >= *reg_pointer_array[rt_i]){
        PC = (uint16_t)*reg_pointer_array[rm_i] & mask; //the casting takes the 16 lower bits.
    }; 
};

//gets register's indexes, performs the jal command as it describes.
void do_jal_command(uint8_t rd_i, uint8_t rm_i){
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) {return;}; //read-only registers.
    uint16_t mask = 4095; //0000111111111111
    reg_pointer_array[rd_i] = PC + 1; 
    PC = (uint16_t)*reg_pointer_array[rm_i] & mask; //the casting takes the 16 lower bits.
};

//gets register's indexes, performs the lw command as it describes.
void do_lw_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) {return;}; //read-only registers.
    uint32_t mem_index = (uint32_t)((int32_t)*reg_pointer_array[rs_i] + (int32_t)*reg_pointer_array[rt_i]); 
    reg_pointer_array[rd_i] = (uint32_t)((int32_t)dmem_array[mem_index] + (int32_t)*reg_pointer_array[rm_i]);
};

//gets register's indexes, performs the sw command as it describes.
void do_sw_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    uint32_t mem_index = (uint32_t)((int32_t)*reg_pointer_array[rs_i] + (int32_t)*reg_pointer_array[rt_i]); 
    uint32_t val = *reg_pointer_array[rm_i] + *reg_pointer_array[rd_i];
    dmem_array[mem_index] = val;
};

int IN_ISR; //just for compilation. delete when combining.

//gets register's indexes, performs the reti command as it describes.
void do_reti_command(){
    PC = *IO_reg_pointer_array[7];
    IN_ISR = 0;
};

//gets register's indexes, performs the in command as it describes.
int do_in_command(uint8_t rd_i, uint8_t rs_i, uint8_t rt_i){
    if (rd_i == 0 || rd_i == 1 || rd_i == 2) {return;}; //read-only registers.
    uint32_t index = (uint32_t)((int32_t)*reg_pointer_array[rs_i] + (int32_t)*reg_pointer_array[rt_i]);
    if (index > 21) {
        perror("Error at in command. R[rs]+R[rt] is to big for an IORegister index.");
        return -1;
    };
    reg_pointer_array[rd_i] = *IO_reg_pointer_array[index];
    return 1;
};

//gets register's indexes, performs the out command as it describes.
int do_out_command(uint8_t rs_i, uint8_t rt_i, uint8_t rm_i){
    uint32_t index = (uint32_t)((int32_t)*reg_pointer_array[rs_i] + (int32_t)*reg_pointer_array[rt_i]);
    if (index > 21) {
        perror("Error at out command. R[rs]+R[rt] is to big for an IORegister index.");
        return -1;
    };
    IO_reg_pointer_array[index] = *reg_pointer_array[rm_i];
    return 1;
};

//gets the instruction struct. calls the right function to commit it, with the right parameters.
//returns -1 in error, 1 in success and 0 if the command is halt.
int commit_the_instruction(Instruction inst){
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
            return do_out_command(inst.rd, inst.rs, inst.rt);
        case 21:
            return 0;
        default:
            perror("Opcode is not recognize (not in 0-21). error in commit_the_instruction\n");
            return -1;
    };
};

int main(int argc, char *argv[]) {
    set_up_files(argv + 2);
}
