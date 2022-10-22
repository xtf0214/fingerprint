#include <Arduino.h>
#include <Servo.h>                // 舵机
#include <Keypad.h>               // 矩阵键盘
#include <U8glib.h>               // u8g库 用于0.96 OLED IIC显示器
#include <Adafruit_Fingerprint.h> // AS608指纹
#include <DHT.h>                  // 温湿度传感
#include "font.h"                 // 调用同目录下的字库

#define buzzerPin 13
Servo myservo;       // 舵机使用
const byte ROWS = 2; // 矩阵键盘行数
const byte COLS = 4; // 矩阵键盘列数
// 按键定义
char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3', '4'},
    {'5', '6', '7', '8'}};
byte rowPins[ROWS] = {8, 9};       // R4 3
byte colPins[COLS] = {5, 4, 3, 2}; // C4 3 2 1
Keypad customKeypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
DHT dht(7, DHT11);                                               // 温湿度data接脚
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0); // I2C / TWI
SoftwareSerial mySerial(11, 12);                                 // 新建一个名为mySerial的软串口 并将11号引脚作为RX端 12号引脚作为TX端
Adafruit_Fingerprint finger(&mySerial);                          // 指纹模块使用

int knockmode = 1;
u8 ID[64] = {0}, st = 0;
int row = 0;
u8 judge;
u16 q = 1024, t, h; //累计开门次数、温度、湿度
u8 key_num = 0;
u8 Angle = 75; //舵机默认开合角度   主页面按下K4进入设置舵机转动角度功能

void buzzer(u16 time)
{
    digitalWrite(buzzerPin, HIGH);
    delay(time);
    digitalWrite(buzzerPin, LOW);
    delay(time);
}
void MG90S() // MG90S舵机PWM脉冲
{
    myservo.attach(10);
    if (knockmode == 1)
    {
        u8g.firstPage();
        do
        {
            u8g.drawXBMP(0, 0, 128, 64, State23);
            char id[10];
            sprintf(id,"id=%02d", (int)judge);
            u8g.drawStr(0, 63, id);
        } while (u8g.nextPage());
        buzzer(1000);
        myservo.writeMicroseconds(500);
        delay(800);
        myservo.writeMicroseconds(1500);
        knockmode = -knockmode;
    }
    else if (knockmode == -1)
    {
        u8g.firstPage();
        do
        {
            u8g.drawXBMP(0, 0, 128, 64, State24);
            char id[10];
            sprintf(id,"id=%02d", (int)judge);
            u8g.drawStr(0, 63, id);
        } while (u8g.nextPage());
        buzzer(1000);
        myservo.writeMicroseconds(2500);
        delay(725);
        myservo.writeMicroseconds(1500);
        knockmode = -knockmode;
    }
}

/* -------------
 *  添加指纹
 * -------------*/
void Add_FR()
{
    u8 ID_NUM;
    for (u8 i = st; i < 64; i++)
        if (ID[i] == 0)
        {
            st = i;
            ID_NUM = i;
            break;
        }

    u8 i, ensure, processnum = 0;
    int j = 0;
    char str2[10];
    key_num = 0;
    // key_num=key_scan(0);
    // if(key_num==1)
    //   goto MENU;
    while (1)
    {
        switch (processnum)
        {
        case 0:
            i++;
            u8g.firstPage();
            do
            {
                u8g.drawXBMP(44, 12, 40, 40, State5); /* 字串 请按手指   64x16  */
            } while (u8g.nextPage());
            while ((ensure = finger.getImage()) != 0x00 && (key_num = customKeypad.getKey()) != '1')
                ;
            if (key_num == '1')
                goto MENU;
            if (ensure == FINGERPRINT_OK)
            {
                ensure = finger.image2Tz(1); //生成特征
                if (ensure == FINGERPRINT_OK)
                {
                    u8g.firstPage();
                    do
                    {
                        u8g.drawXBMP(44, 12, 40, 40, State6); /* 字串 指纹正常  64x16  */
                    } while (u8g.nextPage());
                    // Serial.println(" 000 is true");
                    i = 0;
                    processnum = 1; //跳到第二步
                }
            }
            break;

        case 1:
            i++;
            u8g.firstPage();
            do
            {
                u8g.drawXBMP(44, 12, 40, 40, State7); /* 字串 再按一次   64x16  */
            } while (u8g.nextPage());
            ensure = finger.getImage();
            if (ensure == FINGERPRINT_OK)
            {
                ensure = finger.image2Tz(2); //生成特征
                if (ensure == FINGERPRINT_OK)
                {
                    u8g.firstPage();
                    do
                    {
                        u8g.drawXBMP(44, 12, 40, 40, State6); /* 字串 指纹正常  64x16  */
                    } while (u8g.nextPage());
                    i = 0;
                    processnum = 2; //跳到第三步
                }
            }
            break;

        case 2:
            u8g.firstPage();
            do
            {
                u8g.drawXBMP(44, 12, 40, 40, State8); /* 字串 创建模板   64x16  */
            } while (u8g.nextPage());
            ensure = finger.createModel();
            if (ensure == FINGERPRINT_OK)
            {
                u8g.firstPage();
                do
                {
                    u8g.drawXBMP(16, 24, 96, 16, State9); /* 字串 模板创建成功   96x16  */
                } while (u8g.nextPage());
                processnum = 3; //跳到第四步
            }
            else
            {
                u8g.firstPage();
                do
                {
                    u8g.drawXBMP(16, 24, 96, 16, State10); /* 字串 模板创建失败   96x16  */
                } while (u8g.nextPage());
                i = 0;
                processnum = 0; //跳回第一步
            }
            delay(500);
            break;
        case 3:

            u8g.firstPage();
            do
            {
                u8g.drawXBMP(1, 0, 128, 48, State11);
                /* 字串  按S4加，按S2减 按S3保存 0=< ID <=99 128x48*/
                u8g.setFont(u8g_font_6x10); // 选择字体
                char ID_str[6];
                sprintf(ID_str, "ID=%02d", ID_NUM);
                u8g.drawStr(40, 62, ID_str);
            } while (u8g.nextPage());

            while (key_num != '3')
            {
                key_num = customKeypad.getKey();
                if (key_num == '2')
                {
                    key_num = 0;
                    if (ID_NUM > st)
                    {
                        ID_NUM--;
                        while (ID[ID_NUM] == 1 && ID_NUM > st)
                            ID_NUM--;
                    }
                    if (ID_NUM < 10)
                        sprintf(str2, "ID=0%d", ID_NUM);
                    else
                        sprintf(str2, "ID=%d", ID_NUM);
                    u8g.firstPage();
                    do
                    {
                        u8g.setFont(u8g_font_6x10); // 选择字体
                        u8g.drawXBMP(1, 0, 128, 48, State11);
                        u8g.drawStr(40, 62, str2);
                    } while (u8g.nextPage());
                }
                if (key_num == '4')
                {
                    key_num = 0;
                    if (ID_NUM < 99)
                    {
                        ID_NUM++;
                        while (ID[ID_NUM] == 1)
                            ID_NUM++;
                    }
                    if (ID_NUM < 10)
                        sprintf(str2, "ID=0%d", ID_NUM);
                    else
                        sprintf(str2, "ID=%d", ID_NUM);
                    u8g.firstPage();
                    do
                    {
                        u8g.setFont(u8g_font_6x10);
                        u8g.drawStr(40, 62, str2);
                        u8g.drawXBMP(1, 0, 128, 48, State11);
                    } while (u8g.nextPage());
                }
            }
            key_num = 0;
            ensure = finger.storeModel(ID_NUM); //储存模板
            ID[ID_NUM] = 1;
            if (ensure == 0x00)
            {
                u8g.firstPage();
                do
                {
                    u8g.drawXBMP(16, 24, 96, 16, State12); /* 字串 录入指纹成功   96x16  */
                } while (u8g.nextPage());
                Serial.println("FR receive OK");
                delay(1500);
                return;
            }
            else
            {
                processnum = 0;
            }
            break;
        }
        delay(400);
        if (i == 10) //超过5次没有按手指则退出
        {
            break;
        }
    }
MENU:
    key_num = 0;
}
/* -------------
 *  验证指纹并开锁
 * -------------*/
void Press_FR()
{
    u8 flag = 0;
    u8 ensure;
    u32 i;
    judge = 1;
    char str[20];
    char wendu[15];
    char shidu[15];
    u8g.firstPage();
    do
    {
        u8g.drawXBMP(44, 12, 40, 40, State5); /* 字串 请按手指   64x16  */
    } while (u8g.nextPage());
    while ((ensure = finger.getImage()) != 0x00); //获取图像成功
    while (1)
    {
        if (ensure == 0x00) //获取图像成功
        {
            ensure = finger.image2Tz();
            if (ensure == 0x00) //生成特征成功
            {
                ensure = finger.fingerFastSearch();
                if (ensure == 0x00) //搜索成功
                {
                    int i = 0;
                    key_num = 0;
                    sprintf(str, "ID:%d Score:%d", finger.fingerID, finger.confidence);
                    judge = (int)finger.fingerID; //给id赋值，用来判断是否管理员
                    u8g.firstPage();
                    Serial.print("flag = ");
                    Serial.println(flag);
                    // if(flag == 0){
                    // do
                    //     {
                    //         // 指纹验证成功
                    //         u8g.setFont(u8g_font_6x10);            // 选择字体
                    //         u8g.drawXBMP(16, 16, 96, 16, State13); //显示指纹搜索成功
                    //         u8g.drawStr(1, 46, str);
                    //     } while (u8g.nextPage());
                    //     flag = 0;
                    //     delay(2000);
                    // }
                    MG90S();
                    Serial.println("OPEN");
                    delay(1401);
                    t = dht.readTemperature();
                    h = dht.readHumidity();
                    delay(100);
                    sprintf(wendu, "%d", t);
                    sprintf(shidu, "%d", h);
                    if (judge == 0)
                    {
                        i = 0;

                        Serial.print("judege=");
                        Serial.println(judge);
                        while (key_num != '1' && i < 8)
                        {
                            key_num = customKeypad.getKey();
                            Serial.println('a');
                            u8g.firstPage();
                            do
                            {
                                // 喜迎二十大的画面
                                u8g.setFont(u8g_font_8x13);
                                u8g.drawXBMP(0, 0, 128, 64, BG); /* 图片 主页  128x64  */
                                u8g.drawXBMP(80, 32, 48, 32, welcomeXJP + row);
                                u8g.drawStr(40, 47, wendu);
                                u8g.drawStr(40, 63, shidu);
                            } while (u8g.nextPage());
                            row = (row + 6) % (6 * 64);
                            i++;
                            Serial.println(i);
                            delay(1);
                        }
                        if (key_num == '1')
                        {
                            key_num = 0;
                            break;
                        }
                    }
                RECHECK:
                // 等待验证指纹的部分
                    while ((ensure = finger.getImage()) != 0x00)
                    {
                        u8g.firstPage();
                        do
                        {
                            // 喜迎二十大的图片
                            u8g.setFont(u8g_font_8x13);
                            u8g.drawXBMP(0, 0, 128, 64, BG); /* 图片 主页  128x64  */
                            u8g.drawXBMP(80, 32, 48, 32, welcomeXJP + row);
                            u8g.drawStr(40, 47, wendu);
                            u8g.drawStr(40, 63, shidu);
                        } while (u8g.nextPage());
                        row = (row + 6) % (6 * 64);
                        flag = 1;
                    }
                }
                else
                {
                    u8g.firstPage();
                    do
                    {
                        u8g.drawXBMP(16, 16, 96, 16, State14); // State14_字串 未搜索到指纹 96x16
                    } while (u8g.nextPage());
                    for (u8 i = 0; i < 3; i++)
                        buzzer(150);
                    delay(600);
                    goto RECHECK;
                    // break;
                }
            }
            else
            {
                // ShowErrMessage(ensure);
            }
        }
    }
}

/* -------------
 *  删除指纹
 * ------------- */
void Del_FR()
{
    int flag = 0;
    u8 ensure;
    u8 ID_NUM = 0;
    char str2[10];
    sprintf(str2, "ID=0%d", ID_NUM);
    u8g.firstPage();
    do
    {
        u8g.setFont(u8g_font_6x10);
        u8g.drawStr(44, 62, str2);
        u8g.drawXBMP(1, 0, 128, 48, State15); //显示字模汉字
    } while (u8g.nextPage());

    while (key_num != '3')
    {
        key_num = customKeypad.getKey();
        if (key_num == '2')
        {
            key_num = 0;
            if (ID_NUM > 0)
                ID_NUM--;
            if (ID_NUM < 10)
                sprintf(str2, "ID=0%d", ID_NUM);
            else
                sprintf(str2, "ID=%d", ID_NUM);
            u8g.firstPage();
            do
            {
                u8g.drawStr(44, 62, str2);
            } while (u8g.nextPage());
        }
        if (key_num == '4')
        {
            key_num = 0;
            if (ID_NUM < 99)
                ID_NUM++;
            if (ID_NUM < 10)
                sprintf(str2, "ID=0%d", ID_NUM);
            else
                sprintf(str2, "ID=%d", ID_NUM);
            u8g.firstPage();
            do
            {
                u8g.drawStr(44, 62, str2);

            } while (u8g.nextPage());
        }
        if (key_num == '1')
            goto MENU; //返回主页面
        if (key_num == '5')
        {
        deletemenu:
            key_num = 0;
            u8 ensure;
            judge = 1;
            int j = 0;
            while (j < 3)
            {
                u8g.firstPage();
                do
                {
                    u8g.drawXBMP(44, 12, 40, 40, State5); /* 字串 请按手指   64x16  */
                } while (u8g.nextPage());
                ensure = finger.getImage();
                if (ensure == 0x00) //获取图像成功
                {
                    ensure = finger.image2Tz();
                    if (ensure == 0x00) //生成特征成功
                    {
                        ensure = finger.fingerFastSearch();
                        if (ensure == 0x00) //搜索成功
                        {
                            judge = (int)finger.fingerID;
                            if (judge == 0)
                            {
                                flag == 0 ? ensure = finger.emptyDatabase() : ensure = finger.deleteModel(ID_NUM); //清空指纹库或删除指纹
                                if (ensure == 0)
                                {
                                    if (flag == 0)
                                    {
                                        u8g.firstPage();
                                        do
                                        {
                                            u8g.drawXBMP(8, 16, 80, 16, State17);  //显示字模汉字
                                            u8g.drawXBMP(88, 16, 32, 16, State19); //显示字模汉字
                                        } while (u8g.nextPage());
                                    }
                                    else
                                    {
                                        u8g.firstPage();
                                        do
                                        {
                                            u8g.drawXBMP(16, 16, 64, 16, State18); //显示字模汉字
                                            u8g.drawXBMP(80, 16, 32, 16, State19); //显示字模汉字
                                        } while (u8g.nextPage());
                                    }
                                    delay(1500);
                                    goto MENU; //返回主页面
                                }
                                else
                                {
                                    if (flag == 0)
                                    {
                                        u8g.firstPage();
                                        do
                                        {
                                            u8g.drawXBMP(8, 16, 80, 16, State17);  //显示字模汉字
                                            u8g.drawXBMP(88, 16, 32, 16, State20); //显示字模汉字
                                        } while (u8g.nextPage());
                                    }
                                    else
                                    {
                                        u8g.firstPage();
                                        do
                                        {
                                            u8g.drawXBMP(16, 16, 64, 16, State18); //显示字模汉字
                                            u8g.drawXBMP(80, 16, 32, 16, State20); //显示字模汉字

                                        } while (u8g.nextPage());
                                    }
                                    delay(1500);
                                }
                            }
                        }
                        else //搜索失败
                        {
                            u8g.firstPage();
                            do
                            {
                                u8g.drawXBMP(16, 16, 96, 16, State14); // State14_字串 未搜索到指纹 96x16
                            } while (u8g.nextPage());
                            delay(1000);
                            j++;
                        }
                    }
                    else
                    {
                        // ShowErrMessage(ensure);
                    }
                }
                delay(200);
            }
            goto MENU;
        }
    }
    flag = 1;
    goto deletemenu;
    ensure = finger.deleteModel(ID_NUM); //删除单个指纹
    if (ensure == 0)
    {
        u8g.firstPage();
        do
        {
            u8g.drawXBMP(16, 16, 64, 16, State18); //显示字模汉字
            u8g.drawXBMP(80, 16, 32, 16, State19); //显示字模汉字
        } while (u8g.nextPage());
    }
    else
    {
        u8g.firstPage();
        do
        {
            u8g.drawXBMP(16, 16, 64, 16, State18); //显示字模汉字
            u8g.drawXBMP(80, 16, 32, 16, State20); //显示字模汉字

        } while (u8g.nextPage());
    }
    delay(1500);
MENU:
    key_num = 0;
}

/* -------------
 *  初始页面
 * -------------*/
void MENU()
{
    u8g.firstPage();
    do
    {
        u8g.setFont(u8g_font_8x13);            /* 选择字体 */
        u8g.drawXBMP(26, 0, 80, 16, State2);   /* 字串 添加指纹   64x16  */
        u8g.drawXBMP(26, 16, 80, 16, State3);  /* 字串 删除指纹   64x16  */
        u8g.drawXBMP(26, 32, 80, 16, State4);  /* 字串 验证指纹   64x16  */
        u8g.drawXBMP(34, 48, 64, 16, State21); /*S8下一页*/

    } while (u8g.nextPage());
    Serial.println("一切准备就绪....");
    Serial.println("By Zhangqichao");
}
void MENU2()
{
    u8g.firstPage();
    do
    {
        u8g.setFont(u8g_font_8x13);            /* 选择字体 */
        u8g.drawXBMP(34, 48, 64, 16, State22); /*S7上一页*/

    } while (u8g.nextPage());
}
void setup()
{
    dht.begin();         //开启温湿度
    u8g.begin();         //开启OLED通信
    Serial.begin(9600);  //开启串口通信 波特率9600
    finger.begin(57600); //设置AS608波特率 57600
    pinMode(buzzerPin, OUTPUT);
    MENU();
}

void loop()
{
    key_num = customKeypad.getKey();
    if (key_num == '1') //按下S1键 调用Add_FR()
    {
        key_num = 0;
        Add_FR();
        MENU();
    }
    if (key_num == '2') //按下S2键 调用Del_FR();
    {
        key_num = 0;
        Del_FR();
        MENU();
    }
    if (key_num == '3') //按下S3键 调用Press_FR()
    {
        key_num = 0;
        Press_FR();
        MENU();
    }
    if (key_num == '7') //按下S7键 上一页
    {
        // MG90S();
        key_num = 0;
        MENU();
    }
    if (key_num == '8') //按下S8键 下一页
    {
        MENU2();
        while (key_num != '7')
        {
            key_num = customKeypad.getKey();
        }
    }
}
