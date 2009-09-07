int main()
{
  unsigned int total = proc_pid_cpu(1);
  printf("TOTAL CPU = %u.%u %%\n", total / 100, total % 100);
  return 0;
}
