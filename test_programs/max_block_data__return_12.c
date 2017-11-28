


int max(int x, int y)
{
  int r = ({ int z, xx = x, yy = y; if (xx > yy) z = xx; else z = yy; z; });
  return r;
}


int main()
{
  return max(12, 6);
}



