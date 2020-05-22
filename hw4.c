// $t should be counted across all lines in the file (should not be reset for each line)
// add mod_regs (for %)
// not first operation, neg number <mult or div>
// fix div_regs to take temp_val (by pointer and increment if needed)



/* hw4.c */
/* Vishaal Kasinath (kasinv) */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>


// function to check if a character is an operator character
int isoper(char c);

// function to check if character c exists in array vars
void set_var_reg(char c, int *num_vars, char vars[], char regs[][4]);

// function to return register pseudo var for a given assignment character
char* get_var_reg(char c, int num_vars, char vars[], char regs[][4]);

// parse string aline from position pos, and return numeric value
void get_num_val(char aline[], int *pos, char num_val[], int *flag_neg);

// function to simulate addition of two registers/values
void add_regs(char final_reg[], char first_reg[], char next_reg[]);

// function to simulate subtraction of two registers/values
void sub_regs(char final_reg[], char first_reg[], char next_reg[], 
                    int first_neg,   int next_neg);

// function to simulate multiplication of two registers/values
void mult_regs(char final_reg[], char first_reg[], char next_reg[], 
                       int first_neg,   int next_neg,     int *temp_val);

// function to simulate division of two registers/values
void div_regs(char final_reg[], char first_reg[], char next_reg[], 
                 int first_neg,   int next_neg,     int *lcount, int *temp_val);

// function to simulate modulus of division of two registers/values
void mod_regs(char final_reg[], char first_reg[], char next_reg[], 
                 int first_neg,   int next_neg,     int *temp_val);

int main(int argc, char* argv[]) {

 	// check for valid # of input arguments
    if (argc < 2) {
        fprintf(stderr, "Invalid usage\n");
        return EXIT_FAILURE; 
    }

    // open file and check if valid
    char file_name[128];
    strcpy(file_name, argv[1]);

    FILE *in_file;                    
    in_file = fopen(file_name, "r");    

    if (!in_file) {
        fprintf(stderr, "Error: Cannot open file!\n");
        return EXIT_FAILURE;
    } 


    char line[256];            // array of chars in line
    char nspline[256];         // array of chars in line w/o spaces
    int num_vars = 0;          // number of variables

    char vars[8];              // array of variablles (max 8)
    char regs[8][4];           // array of registers $s0 to $s7

    char first_reg[12];     	// first register ($t0)  
    char next_reg[12];         	// second register ($t1)
    char final_reg[12];         // final register ($s0)
    char pfinal_reg[12];		// final register of previous line

    int lcount = 0;             

    int  temp_val = 0;          // intialize temp_val for $t<n>, across all lines

  // read one line at a time until end of file
    while (fgets(line, sizeof(line), in_file)) {            

        int i = 0;
        int j = 0;
        while (line[i] != ';' && line[i] != '\0') {         
            if (isspace(line[i]))                          
                i++;
            else{
                nspline[j] = line[i];
                i++; j++;
            }
        }

        line[i+1] = '\0';   
        nspline[j] = '\0';


        // check if blank line
        if (nspline[0] == '\0' || nspline[0] == '\n')
            continue;
 
        // get number of operations
        int num_oper = 0;       

        for (int j = 0; j < 256; j++){
	        if (nspline[j] == ';' || nspline[j] == '\0')		// break if end of line
	       	break;

            if (nspline[2] == '-' && isdigit(nspline[j+1])) 	// skip if negative number    
                continue;                 

            if (isoper(nspline[j])){                         
                if ( nspline[j] == '-' && (nspline[j-1] == '*' || nspline[j-1] == '/' || nspline[j-1] == '-') )
                    j++;
                else
                    num_oper++;
            }
        }

        // get and store variable, set register
    	for (int j = 0; j < 256; j++){
	        if (nspline[j] == ';' || nspline[j] == '\0') break;

			if (isalpha(nspline[j])) 
				set_var_reg(nspline[j], &num_vars, vars, regs); 
        }

        // print line
        printf("# %s\n", line);

        // print assignment 
    	if (num_oper == 0){
	        strcpy(first_reg, get_var_reg(nspline[0], num_vars, vars, regs) );

		    int pos = 2, is_neg = 0;
            get_num_val(nspline, &pos, next_reg, &is_neg);
            printf ("li %s,%s\n", first_reg, next_reg);

            continue;
    	}

        int pos = 2;        
        for (int j = 0; j < num_oper; j++) {

            if (j != num_oper-1){
            	strcpy(pfinal_reg, final_reg);
                sprintf(final_reg, "$t%d", (temp_val % 10) );
                temp_val++;
            }
            else{
                strcpy(pfinal_reg, final_reg);
                strcpy(final_reg, get_var_reg(nspline[0], num_vars, vars, regs) );
            }

            int first_neg = 0, next_neg = 0;
            if (j > 0)
                strcpy(first_reg, pfinal_reg);
            else {
                if ((nspline[pos] == '-') || isdigit(nspline[pos]) ){
                    get_num_val(nspline, &pos, first_reg, &first_neg);
                }
                else{
                    strcpy(first_reg, get_var_reg(nspline[pos], num_vars, vars, regs) );
                    pos++;
                }
            }

            char operation;
            operation = nspline[pos];
            pos++;

            if ((nspline[pos] == '-') || isdigit(nspline[pos]) ){
                get_num_val(nspline, &pos, next_reg, &next_neg);
            }
            else{
                strcpy(next_reg, get_var_reg(nspline[pos], num_vars, vars, regs));
                pos++;
            }


            // special case handling for next_reg = negative number and oper = mult or div
            // do a li, final_reg, negative_number (which is our next_reg)
            // then set next_reg = the previous final_reg
            // then set the final_reg to be the next temp_val
            if (j != num_oper-1 && (isdigit(next_reg[0]) || next_reg[0] == '-') && (operation == '*' || operation == '/') && next_neg == 1){
                // char temp_reg[12];
            	printf("li %s,%s\n", final_reg, next_reg);

                // set next_reg = previous final_reg
                // set new final_reg to $t<n>
                strcpy(next_reg, final_reg);
                sprintf(final_reg, "$t%d", temp_val % 10);
            }

            if (operation == '+')
                add_regs(final_reg, first_reg, next_reg);
            else if (operation == '-')
                sub_regs(final_reg, first_reg, next_reg, first_neg, next_neg);
            else if (operation == '*')
                mult_regs(final_reg, first_reg, next_reg, first_neg, next_neg, &temp_val);
            else if (operation == '/')
                div_regs(final_reg, first_reg, next_reg, first_neg, next_neg, &lcount, &temp_val);
            else if (operation == '%')
                mod_regs(final_reg, first_reg, next_reg, first_neg, next_neg, &temp_val);

        }
    }
    fclose(in_file);

    return EXIT_SUCCESS;
}

/* FUNCTIONS */

// function to check if operation
int isoper(char c) {
  if (c == '+' || c == '-' || c == '*'  || c == '/' || c =='%')
    return 1;
  else
    return 0;
}


// function to check if character c exists in array vars
void set_var_reg(char c, int *num_vars, char vars[], char regs[][4]){

    int cnum = *num_vars;
    for (int i=0; i < cnum; i++){
        if (vars[i] == c){              // if char c already exists in vars[]
            return;                     // return silently
        }
    }

    // add new char c to list
    vars[cnum] = c;             
    sprintf(regs[cnum], "$s%d", cnum) ;
    (*num_vars)++;
}


// function to return register for a given variable
char* get_var_reg(char c, int num_vars, char vars[], char regs[][4]){

    for (int i=0; i < num_vars; i++){
        if (vars[i] == c)
            return regs[i];
    }

    return 0;

}

// parse string and return numeric value
void get_num_val(char aline[], int *pos, char num_val[], int *flag_neg) {

    int m = *pos;
    int n = 0, neg = 0;

    if (aline[m] == '-'){
        neg = 1;
        m++;
    }

    while (isdigit(aline[m]) ){
        n = n*10 + (aline[m]-'0');
        m++;
    }
    if (neg == 1) {
        n = n * -1;
        *flag_neg = 1;
    }

    sprintf(num_val, "%d", n);
    *pos = m;

    return;
}

// function to add of two values
void add_regs(char final_reg[], char first_reg[], char next_reg[]){

    if (isdigit(first_reg[0]) || isdigit(next_reg[0]) )                             // adding two numbers
        printf("addi %s,%s,%s\n",final_reg, first_reg, next_reg);

    else if (first_reg[0] == '-' && isdigit(first_reg[1]) && next_reg[0] =='$'){    // neg num + variable
        printf("addi %s,%s,%s\n",final_reg, next_reg, first_reg);
    }

    else if (next_reg[0] == '-' && isdigit(next_reg[1]) && first_reg[0] == '$')     // variable + neg num
        printf("addi %s,%s,%s\n",final_reg, first_reg, next_reg);

    else if (isdigit(first_reg[0]) && next_reg[0] =='$'){                           // positive num + variable
        printf("addi %s,%s,%s\n",final_reg, first_reg, next_reg);
    }

    else if (isdigit(next_reg[0]) && first_reg[0] == '$')                           // variable + positive num
        printf("addi %s,%s,%s\n",final_reg, first_reg, next_reg);

    else
        printf("add %s,%s,%s\n",final_reg, first_reg, next_reg);

    return;
}

// function to subtract two values
void sub_regs(char final_reg[], char first_reg[], char next_reg[], int first_neg, int next_neg){

    // both are variable registers
    if (first_reg[0] == '$' && next_reg[0] == '$'){
        printf("sub %s,%s,%s\n", final_reg, first_reg, next_reg);
        return;
    }

    // negative numbers
    if (first_neg == 1 && next_neg == 0){
        printf("addi %s,-%s,-%s\n",final_reg, first_reg, next_reg);
        return;
    }

    if (first_neg == 0 && next_neg == 0){
        printf("addi %s,%s,-%s\n",final_reg, first_reg, next_reg);
        return;
    }

    return;
}

// function to multiply two values
void mult_regs(char final_reg[], char first_reg[], char next_reg[], int first_neg, int next_neg, int *temp_val){

    // two variavles
    if (first_reg[0] == '$' && next_reg[0] == '$'){
        printf("mult %s,%s\n", first_reg,next_reg);
        printf("mflo %s\n", final_reg);
        return;
    }


    int temp;
    sscanf(next_reg, "%d", &temp);

    if ( temp == 0 ){
        printf("li %s,0\n", final_reg);
        return;
    }

    if ( temp == 1 ){
        printf("move $t%d,%s\n", *temp_val,first_reg);
        printf("move %s,$t%d\n", final_reg, *temp_val);
        (*temp_val)++;
        return;
    }

    if ( temp == -1 ){
        printf("move $t%d,%s\n", *temp_val,first_reg);
        printf("sub %s,$zero,$t%d\n", final_reg, *temp_val);
        (*temp_val)++;
        return;
    }

    if (isdigit(next_reg[0]) || (next_neg == 1 && isdigit(next_reg[1]) )){
        int temp;
        sscanf(next_reg, "%d", &temp);
       
        if (next_neg == 1)
            temp = temp * -1;

        int bin[32];
        int n = 0;
        while (temp > 1){
            bin[n] = temp % 2;
            temp = temp / 2;
            n++;
        }

        for (int i=0; i < n; i++){
            int pow2 = n-i;
            if (bin[i] == 1){
                printf("sll $t%d,%s,%d\n", (*temp_val % 10), first_reg, pow2);            
                if (i == 0)
                    printf("move $t%d,$t%d\n", ( (*temp_val+1) % 10), (*temp_val % 10) );
                else
                    printf("add $t%d,$t%d,$t%d\n", ( (*temp_val+1) % 10), ( (*temp_val+1) % 10), (*temp_val % 10) );
            }
        }
        printf("add $t%d,$t%d,%s\n", ( (*temp_val+1) % 10), ( (*temp_val+1) % 10), first_reg );

        if (next_neg == 1)
            printf("sub %s,$zero,$t%d\n", final_reg, ( (*temp_val+1) % 10) );
        else
            printf("move %s,$t%d\n", final_reg, ( (*temp_val+1) % 10) );

        (*temp_val)++;
        return;
    }

    // num * variable
    if (isdigit(first_reg[0]) || (first_neg == 1 && isdigit(first_reg[1]) )){
        
        char str[4];
        strcpy(str, first_reg);
        strcpy(first_reg, next_reg);
        strcpy(next_reg, str);

        int neg;
        neg = first_neg;
        first_neg = next_neg;
        next_neg = neg;


        int temp;
        sscanf(next_reg, "%d", &temp);      

        if (next_neg == 1)
            temp = temp * -1;

        int bin[32];
        int n = 0;
        while (temp > 1){
            bin[n] = temp % 2;
            temp = temp / 2;
            n++;
        }

        for (int i=0; i < n; i++){
            int pow2 = n-i;
            if (bin[i] == 1){
                printf("sll $t%d,%s,%d\n", (*temp_val % 10), first_reg, pow2);            
                if (i == 0)
                    printf("move $t%d,$t%d\n", ( (*temp_val+1) % 10), (*temp_val % 10) );
                else
                    printf("add $t%d,$t%d,$t%d\n", ( (*temp_val+1) % 10), ( (*temp_val+1) % 10), (*temp_val % 10) );
            }
        }
        printf("add $t%d,$t%d,%s\n", ( (*temp_val+1) % 10), ( (*temp_val+1) % 10), first_reg );

        if (next_neg == 1)
            printf("sub %s,$zero,$t%d\n", final_reg, ( (*temp_val+1) % 10) );
        else
            printf("move %s,$t%d\n", final_reg, ( (*temp_val+1) % 10) );

        strcpy(str, next_reg);
        strcpy(next_reg, first_reg);
        strcpy(first_reg, str);

        neg = next_neg;
        next_neg = first_neg;
        first_neg = neg;

        (*temp_val)++;

        return;
    }

    return;
}

// function to divide two values
void div_regs(char final_reg[], char first_reg[], char next_reg[], int first_neg, int next_neg, int *lcount, int *temp_val) {

	// two registers
    if (first_reg[0] == '$' && next_reg[0] == '$'){
        printf("div %s,%s\n", first_reg,next_reg);
        printf("mflo %s\n", final_reg);

        if (final_reg[1] == 't')
            (*temp_val)++;
        return;
    }

    // divide by 1
    if (next_reg[0] == '1'){
        printf("move %s,%s\n", final_reg, first_reg);
        return;
    }

    int temp;
    sscanf(next_reg, "%d", &temp);

    if (temp == 1){
        printf("move %s,%s\n",final_reg, first_reg);
        return;
    }

    if (next_neg == 1 && temp == -1){
        printf("sub %s,$zero,%s\n",final_reg, first_reg);
        return;
    }

    if (isdigit(next_reg[0]) || (next_neg == 1 && isdigit(next_reg[1]) )){

        if (next_neg == 1)
            temp = temp * -1;

        int bin[32];
        int n = 0;
        while (temp > 1){
            bin[n] = temp % 2;
            temp = temp / 2;
            n++;
        }
        
        int bin_sum = temp;
        for (int i=0; i<n; i++)
            bin_sum +=bin[i];

        if (first_neg == 0 && bin_sum == 1){
            printf("bltz %s,L%d\n", first_reg, *lcount);
            printf("srl %s,%s,%d\n", final_reg, first_reg, n);

            if (next_neg == 1)
                printf("sub %s,$zero,%s\n", final_reg, final_reg);

            printf("j L%d\n", (*lcount)+1);
            printf("L%d:\n", *lcount);

            char temp_reg[12];
            sprintf(temp_reg, "$t%d", *temp_val);
            (*temp_val)++;

            printf("li %s,%s\n", temp_reg, next_reg);
            div_regs(final_reg, first_reg, temp_reg, first_neg, next_neg, lcount, temp_val);
            printf("L%d:\n", (*lcount)+1 );

            (*lcount) = (*lcount)+2; 
        }

        return;
    }

    return;
}


void mod_regs(char final_reg[], char first_reg[], char next_reg[], int first_neg, int next_neg, int *temp_val){

// two registers
    if (first_reg[0] == '$' && next_reg[0] == '$'){
        printf("div %s,%s\n", first_reg,next_reg);
        printf("mfhi %s\n", final_reg);

        if (final_reg[1] == 't')
            (*temp_val)++;
        return;
    }

    int temp;
    sscanf(next_reg, "%d", &temp);

    if (temp == 1){
        printf("move %s,%s\n",final_reg, first_reg);
        return;
    }

    if (next_neg == 1 && temp == -1){
        printf("sub %s,$zero,%s\n",final_reg, first_reg);
        return;
    }

    if (isdigit(next_reg[0]) || (next_neg == 1 && isdigit(next_reg[1]) )){

        if (next_neg == 1)
            temp = temp * -1;

        char temp_reg[12];
        sprintf(temp_reg, "$t%d", *temp_val);
        (*temp_val)++;

        printf("li %s,%s\n", temp_reg, next_reg);
        mod_regs(final_reg, first_reg, temp_reg, first_neg, next_neg, temp_val);

        return;
    }

    return;
}
