
static long mapRound(long x, long in_min,long in_max, long out_min, long out_max)
{
        return ((x - in_min) * (out_max - out_min) + (in_max - in_min) / 2) / (in_max - in_min) + out_min;
}

static double mapDouble(double x, double in_min, double in_max, double out_min, double out_max)
{
        return (double)((x - in_min) * ((double)out_max - (double)out_min)  / (in_max - in_min) + (double)out_min);
}
