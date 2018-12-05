/*
 * 去暗角
*/
#include <cmath>
#include <QImage>

static QImage g_image, g_new_image;

//    按论文中公式18得条件判断是否是合理的参数
bool IsValidABC_Original(float A, float B, float C)
{
    const int MAX_BRIGHTNESS_MULTIPLICATION = 3;
    if ((1 + A + B + C) > MAX_BRIGHTNESS_MULTIPLICATION)    return false;        //    当r==1时，出现最大的亮度调整
    if (C == 0)
    {
        if (A <= 0 || (A + 2 * B <= 0))            //    如果C==0，则根据公式(15)知，当r==0时，A必须大于0，而当r==1时，A+2B必须大于0
            return false;
    }
    else
    {
        float D = 4 * B * B - 12 * A * C;
        float qMins = (-2 * B - sqrtf(D)) / (6 * C);        //    公式(17)
        float qPlus = (-2 * B + sqrtf(D)) / (6 * C);
        if (C < 0)
        {
            if (D >= 0)
            {
                if (qMins > 0 || qPlus < 1)
                    return false;
            }
            else
                return false;
        }
        else
        {
            if (D >= 0)
            {
                if (!((qMins <= 0 && qPlus <= 0) || (qMins >= 1 && qPlus >= 1)))
                    return false;
            }
        }
    }
    return true;
}

float CalculateEntropyFromHistgram_Original(float Histgram[], int Length);
//    计算不同参数修复后的图像的整体对数熵
float CalculateEntropyFromImage(unsigned char *Src, int Width, int Height, float A, float B, float C, int CenterX, int CenterY)
{
    float Histgram[256] = { 0 };
    float Invert = 1.0f / (CenterX * CenterX + CenterY * CenterY + 1);
    float Mul_Factor = 32.f / log(256.0f);     //调小256！！！！
    for (int Y = 0; Y < Height; Y++)
    {
        //unsigned char *LinePS = Src + Y * Width * 4;
        int SquareY = (Y - CenterY) * (Y - CenterY);
        for (int X = 0; X < Width; X++)
        {
            //int Intensity = (LinePS[0] + (LinePS[1] << 1) + LinePS[2]) >> 2;         //    公式(2)

            int Intensity = (QColor(g_image.pixel(X,Y)).blue() +QColor(g_image.pixel(X,Y)).green() + QColor(g_image.pixel(X,Y)).red())/3;

            int RadiusSqua2 = (X - CenterX) * (X - CenterX) + SquareY;
            float R = RadiusSqua2 * Invert;                        //    公式(12)
            float Gain = 1 + (R * (A + R * (B + R * C)));            //    gain = 1 + a * r^2 + b * r^4 + c * r^6 ，公式(11)
            float Correction = Gain * Intensity;                    //    直接校正后的结果值
            if (Correction >= 255)
            {
                Correction = 255;                //    It is possible that, due to local intensity increases applied by devignetting, the corrected image intensity range exceeds 255.
                Histgram[255]++;                //    In this case the algorithm simply adds histogram bins at the upper end without rescaling the distribution,
            }
            else
            {
                float Pos = Mul_Factor * log(Correction + 1);    //    公式(6)
                int Int = int(Pos);
                Histgram[Int] += 1 - (Pos - Int);    //    公式(7)
                Histgram[Int + 1] += Pos - Int;
            }
            //LinePS += 4;
        }
    }
    float TempHist[256 + 2 * 4];            //    SmoothRadius = 4
    TempHist[0] = Histgram[4];                TempHist[1] = Histgram[3];
    TempHist[2] = Histgram[2];                TempHist[3] = Histgram[1];
    TempHist[260] = Histgram[254];            TempHist[261] = Histgram[253];
    TempHist[262] = Histgram[252];            TempHist[263] = Histgram[251];
    memcpy(TempHist + 4, Histgram, 256 * sizeof(float));

    for (int X = 0; X < 256; X++)            //    公式(8),进行一个平滑操作
        Histgram[X] = (TempHist[X] +  TempHist[X + 1] +  TempHist[X + 2] +  TempHist[X + 3] +  TempHist[X + 4] +  TempHist[X + 5] +  TempHist[X + 6] +  TempHist[X + 7] + TempHist[X + 8]) / 9.f;

    return CalculateEntropyFromHistgram_Original(Histgram, 256);
}


//    从直方图中计算熵值,原论文中直方图肯定是浮点数
float CalculateEntropyFromHistgram_Original(float Histgram[], int Length)
{
    float Sum = 0;
    for (int X = 0; X < Length; X++)
    {
        Sum += Histgram[X];
    }
    float Entropy = 0;
    for (int X = 0; X < Length; X++)
    {
        if (Histgram[X] == 0) continue;
        float p = (float)Histgram[X] / Sum;
        Entropy += p * logf(p);
    }
    return -Entropy;
}

#define  ClampToByte(  v )  (((unsigned)int(v)) <(255) ? (v) : (v < 0) ? (0) : (255))
int Devignetting_Original(unsigned char *Src, int Width, int Height);

void ToolDark(QImage image_old, QImage &ret)
{
    unsigned char *Src = image_old.bits();

    g_image = image_old;
    g_new_image = image_old;
    Devignetting_Original(Src, image_old.width(), image_old.height());

    ret = g_new_image;
}

int Devignetting_Original(unsigned char *Src, int Width, int Height)
{
    const float Step = 0.5f;  // 粗选A\B\C三个变量的步长

    float SmallestEntropy = 1000000000000.0f;
    float A = 0, B = 0, C = 0;
    int CenterX = Width / 2, CenterY = Height / 2;  // 中心就默认为图片中心

    for (int X = -10; X <= 10; X++)  // 多次测试，表面最优的A\B\C的范围均在[-2,2]之间
    {
        for (int Y = -10; Y <= 10; Y++)
        {
            for (int Z = -10; Z <= 10; Z++)
            {
                if (IsValidABC_Original(X * Step, Y * Step, Z * Step) == true)  // 判断这个组合时候有效
                {
                    float Entropy = CalculateEntropyFromImage(Src, Width, Height, X * Step, Y * Step, Z * Step, CenterX, CenterY);
                    if (Entropy < SmallestEntropy)  // 取熵值最小的
                    {
                        A = X * Step;
                        B = Y * Step;
                        C = Z * Step;
                        SmallestEntropy = Entropy;
                    }
                }
            }
        }
    }

    float Invert = 1.0 / (CenterX * CenterX + CenterY * CenterY + 1);

    int red,green,blue;
    for (int Y = 0; Y < Height; Y++)
    {
        int SquareY = (Y - CenterY) * (Y - CenterY);
        for (int X = 0; X < Width; X++)
        {
            int RadiusSqua2 = (X - CenterX) * (X - CenterX) + SquareY;
            float R2 = RadiusSqua2 * Invert;                                    //    公式12
            float Gain = 1 + (R2 * (A + R2 * (B + R2 * C)));                    //    公式11

            red=QColor(g_image.pixel(X,Y)).red()*Gain>255?255:QColor(g_image.pixel(X,Y)).red()*Gain;
            green=QColor(g_image.pixel(X,Y)).green()*Gain>255?255:QColor(g_image.pixel(X,Y)).green()*Gain;
            blue=QColor(g_image.pixel(X,Y)).blue()*Gain>255?255:QColor(g_image.pixel(X,Y)).blue()*Gain;
            g_new_image.setPixel(X,Y,
                                 qRgb(red,green,blue));

        }
    }
    return 1;
}
