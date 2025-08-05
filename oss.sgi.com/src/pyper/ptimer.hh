
class PyperTimer
{
  public: 
    PyperTimer()
    {
    }
    ~PyperTimer()
    {
    }
    float TimeSinceProgramStart(void) const
    {
      struct timeval time;
      if (gettimeofday(&time, 0))
        puts("gettimeofday() failed");
  printf("%d sec, %d usec\n", time.tv_sec, time.tv_usec);
  double component1 = time.tv_sec;
  double component2 = USEC_TO_SEC((double) time.tv_usec);
  timestamp = component1 + component2;
  printf("timestamp = %f\n", timestamp);

    }
  protected:
    static struct timeval start;
};
