int compress(char* chars, int charsSize) {
        if (chars == NULL)
        return 0;

    if (charsSize == 1)
        return charsSize;

    int k = 0;
    int count = 1;
    int i = 0;
    int q = 0;
    int num = 0;
    while (i < charsSize) {
        while ( (i > 0) && (chars[i] == chars[i - 1])) {
            count++;
            i++;
        }
        if (count > 1) {
             // write the number
            num = count;
             while (num > 9) {
                 q = num/10;
                 chars[k++] = '0' + q;
                 num = num%10;
             }
             chars[k++] = '0' + num;
             if (i < charsSize)
                chars[k++] = chars[i];
        }
        if (count < 2) {
            chars[k++] = chars[i];
        }
        count = 1;
        i++;
    }
    chars[k] = '\0';
    return k;

}
