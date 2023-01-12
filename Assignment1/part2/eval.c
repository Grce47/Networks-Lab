#include <stdio.h>
#include <string.h>
#include <stdlib.h>

double do_op(double ele1, char op, double ele2, int *has_error, char *res)
{
    const double ZERO = 0;
    double ans = 0;
    switch (op)
    {
    case '+':
        ans = ele1 + ele2;
        break;
    case '-':
        ans = ele1 - ele2;
        break;
    case '*':
        ans = ele1 * ele2;
        break;
    case '/':
        if (ele2 == ZERO)
        {
            *has_error = 1;
            strcpy(res, "Error Message: Cant Divide by Zero");
        }
        else
            ans = ele1 / ele2;
        break;
    default:
        *has_error = 1;
        strcpy(res, "Error Message: Invalid Operation");
        break;
    }
    return ans;
}

void eval(char *buf, char *res)
{
    const int SUB_LEN = 25;
    char substr[SUB_LEN], *tmp_ptr;
    double ans = 0, prev_ans = 0;
    int has_prev = 0, bracket_started = 0, has_error = 0;
    char op = '+', prev_op = '+';
    for (int i = 0; buf[i] != '\0'; i++)
    {
        if (has_error)
            break;
        if (buf[i] == ' ')
            continue;
        if (buf[i] == '(')
        {
            bracket_started = 1;
            prev_ans = ans;
            ans = 0;
            has_prev = 1;
            prev_op = op;
            op = '+';
        }
        else if (buf[i] == ')')
        {
            bracket_started = 0;
            ans = do_op(prev_ans, prev_op, ans, &has_error, res);
            op = '+';
            has_prev = 0;
        }
        else if (buf[i] == '+')
        {
            op = '+';
        }
        else if (buf[i] == '-')
        {
            op = '-';
        }
        else if (buf[i] == '*')
        {
            op = '*';
        }
        else if (buf[i] == '/')
        {
            op = '/';
        }
        else
        {
            int len = 0, points = 0;
            for (int j = i; buf[j] != '\0'; j++)
            {
                int ascii = buf[j] - '0';
                if (buf[j] == ')' || buf[j] == '+' || buf[j] == '-' || buf[j] == '*' || buf[j] == '/')
                    break;
                if (!(buf[j] == '.' || (ascii >= 0 && ascii <= 9) || buf[j] == ' '))
                {
                    has_error = 1;
                    strcpy(res, "Error Message: Number Error");
                    break;
                }
                if (buf[j] == '.')
                {
                    points++;
                    len++;
                }
                else if (ascii >= 0 && ascii <= 9)
                {
                    len++;
                }
                else if (buf[j] == ' ')
                {
                    len++;
                }
                else
                    break;
            }
            if (points >= 2)
            {
                has_error = 1;
                strcpy(res, "Error Message: Number Error");
                break;
            }
            for (int j = 0; j < SUB_LEN; j++)
                substr[j] = '\0';
            stpncpy(substr, buf + i, len);
            double number = strtod(substr, &tmp_ptr);
            ans = do_op(ans, op, number, &has_error, res);
            i += len - 1;
        }
    }
    if (bracket_started)
    {
        has_error = 1;
        strcpy(res, "Error Message: Unbalanced Brackets");
    }
    if (has_error == 0)
    {
        sprintf(res, "%f", ans);
    }
}

int main()
{
    char buf[500], res[500];
    while (1)
    {
        for (int i = 0; i < 500; i++)
            buf[i] = '\0';
        scanf(" %[^\n]", buf);
        // printf("%s\n",buf);
        if (buf[0] == '-' && buf[1] == '1')
        {
            int flag = 0;
            for (int i = 2; buf[i] != '\0'; i++)
                if (buf[i] != ' ')
                    flag = 1;
            if (!flag)
                break;
        }
        for (int i = 0; i < 500; i++)
            res[i] = '\0';
        eval(buf, res);
        printf("%s\n", res);
    }
    return 0;
}