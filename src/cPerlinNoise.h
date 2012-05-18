#ifndef CPERLINNOISE_H
#define CPERLINNOISE_H

namespace Di
{
    class cPerlinNoise
    {
        public:
            cPerlinNoise() {}
            virtual ~cPerlinNoise() {}

            int getPerlinNoise(float x,float y,float factor, float persistence = 0.2f, float frequency = 0.1f, float amplitude = 1.0f)
            {
               float total = 0;
               // ��� ����� ����� ����� � ������ �������� ���� cosf(sqrtf(2))*3.14f
               // ������� ����� ���� ������� � ��������� ��� ���������
               //float persistence = 0.5f;

               // ����������������� � ����� ����������, ���������� �������
               // �������� sqrtf(3.14f)*0.25f ��� ���-�� ��������� ��� ��������� J)
               //float frequency = 0.05f;
               //float amplitude = 0.70;//���������, � ������ ����������� �� �������� �������������

               // ������ ������ �����������, ����� ������ �� ���� ������ �����������
               // (�� ���� ������ ��� �-��� ���� ����������?)

               x+= (factor);
               y+= (factor);

               // NUM_OCTAVES - ����������, ������� ���������� ����� �����,
               // ��� ������ �����, ��� ����� ���������� ���
               for(int i=0;i<PERLIN_NOISE_OCTAVES;i++)
               {
                   total += CompileNoise(x*frequency, y*frequency) * amplitude;
                   amplitude *= persistence;
                   frequency*=2;
                }
                //����� ����� ��������� �������� �����   �� �����-�� �������
                //��������:
                //total=sqrt(total);
                // total=total*total;
                // total=sqrt(1.0f/float(total));
                //total=255-total;-�.�.� ��� ������� �� ��������� ����������
                total=fabsf(total);
                int res=int(total*255.0f);//�������� ���� � �������� 0-255�
                return res;
            }

        protected:
        private:

            float Noise2D(int x, int y)
            {
              int n = x + y * 57;
              n = (n<<13) ^ n;
              return ( 1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) /
                  1073741824.0f);
            }

            ////////////////////////////////////////////////////////////////////////////
            float SmoothedNoise2D(float x, float y)
            {
                float corners = ( Noise2D(x-1, y-1)+Noise2D(x+1, y-1)+
                     Noise2D(x-1, y+1)+Noise2D(x+1, y+1) ) / 16;
                float sides   = ( Noise2D(x-1, y)  +Noise2D(x+1, y)  +
                     Noise2D(x, y-1)  +Noise2D(x, y+1) ) /  8;
                float center  =  Noise2D(x, y) / 4;
                return corners + sides + center;
              }
            ////////////////////////////////////////////////////////////////////////////

              //�������, ������� ���������� �������� ����������� �
              //������������������ ����
            float CompileNoise(float x, float y) {
                  float int_X    = int(x);//����� ����� �
                  float fractional_X = x - int_X;//����� �� �
            //���������� �
                  float int_Y    = int(y);
                  float fractional_Y = y - int_Y;
              //�������� 4 ���������� ��������
                 float v1 = SmoothedNoise2D(int_X,     int_Y);
                 float v2 = SmoothedNoise2D(int_X + 1, int_Y);
                 float v3 = SmoothedNoise2D(int_X,     int_Y + 1);
                 float v4 = SmoothedNoise2D(int_X + 1, int_Y + 1);
              //������������� �������� 1 � 2 ���� � ���������� ������������ ����� ����
                  float i1 = Cosine_Interpolate(v1 , v2 , fractional_X);
                  float i2 = Cosine_Interpolate(v3 , v4 , fractional_X);
              //� ����������� ���������� ������������ ���� �����
              //�� ���������� ��������-//��������
                  return Cosine_Interpolate(i1 , i2 , fractional_Y);
            }

            float Cosine_Interpolate(float a, float b, float x)
            {
                float ft = x * 3.1415927;
                float f = (1 - cos(ft)) * .5;
                return a*(1-f) + b*f;
            }
    };
}
#endif // CPERLINNOISE_H
