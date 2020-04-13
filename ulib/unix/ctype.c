
int isspace(int c) {
    //"The isspace( ) function shall return non-zero if c is a white-space character;
    //otherwise, it shall return 0."  - POSIX Base Spec, Issue 6 page 647
    char chr = (char)c;
    if(chr == ' ' || chr == '\t' || chr == '\v' || chr == '\n' || chr == '\f' || chr == '\r'){
        //the above is the list of all ASCII-space whitespace character codes
        //see: https://en.wikipedia.org/wiki/Whitespace_character#Unicode
        return 1;
    }
    return 0;
}
