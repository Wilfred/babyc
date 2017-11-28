
int fn1(int a)
{
   return a / -18;
}


int fn2(int a)
{
   int b = -18;
   return a / b;
}

int fn3(int a)
{
   return -72 / -18;
}


int main()
{
   int a = -72;
   return fn1(a) + fn2(a) + fn3(a);
}

