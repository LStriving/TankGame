#define _CRT_SECURE_NO_WARNINGS
#include<graphics.h>
#include<iostream>
#include<conio.h>
#include<mmsystem.h>
#include<fstream>
#include<Windows.h>
#include<ctime>
#include<random>
#include<stdio.h>
#pragma comment(lib,"winmm.lib")
using namespace std;
//颜色的宏定义
#define LIGHTERBLUE 0XFFA500
#define GOLD		0X00D7FF
#define GREY		0XB1B1B1
#define ORANGE		0X00A5FF
#define BRICK		0X1444CF
//判断指定的按键是否按下
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code)&0x8000)?1:0)
//判断指定的按键是否弹起
#define KEY_UP(vk_code) ((GetAsyncKeyState(vk_code)&0x8000)?1:0)
#define Fire 100
///**********map*************
//特定值代表不同的地形
#define Jungle  5
#define Empty  0
#define Ice  9
#define Wall  1
#define Iron  2
#define Home  3
#define DestroyHome  10
const int map_x = 26;				//地图的行数
const int map_y = 26;				//地图的列数
int stage = 1;						//关卡，初始为1
int totalStage = 4;					//所有的关卡数目（目前为4）
char* level = new char[4];			//关卡的另一种记录类型
static int map[map_x][map_y] = {};	//地图数组，记录不同关卡的地图地形

//**************Map/Resource******************
IMAGE background, gameMenuBackground;
IMAGE enemyMark, enemyMark_hide, playerMark1, playerMark2, playerMark1_hide, playerMark2_hide, flag, flag_hide;
IMAGE home[2], home_hide[2], wall, iron, ice, sea[2], jungle, border, jungle_hide;
IMAGE star[4], star_hide[4];
IMAGE stageMenu, gameEnd;
IMAGE boom[5], boom_hide[5];

//************BasicSetting********************
const int unit = 28;				//单位大小,直接调节可改变窗口的大小
const int graphWidth = unit * 34;	//窗口的宽度
const int graphHeight = unit * 26;	//窗口的高度
int gamemode = 0;					//游戏模式：1-单人，2--双人
static long long spendTime;			//用户的游戏时间，不包括暂停时间
const int refleshFreq = 1;			//画面的刷新频率：1毫秒刷新一次
bool gameover = 0;					//判断游戏是否结束
static bool stagechange = 1;		//用来检测关卡是否发生改变

//************Movable thing*******************
enum direction {
	UP = 0, DOWN = 1, RIGHT = 3, LEFT = 2
};			
typedef int Direction;

//******enemy tank******
static int enemyNumber = 14;		//每关敌人的总数
enum class enemyType {				//敌人的类型
	normal=0,moveFaster,shootFaster,
	stronger//2Hp
};		

//*************Bullet***************
void playShootSound() {
	static int counter = 0;
	char play[14] = "";
	char seek[22] = "";
	counter++;
	if (counter == 5)counter = 1;
	sprintf_s(play, sizeof(play), "play shoot%d", counter);
	sprintf_s(seek, sizeof(seek), "seek shoot%d to start", counter);
	mciSendString(seek, NULL, 0, 0);
	mciSendString(play, NULL, 0, 0);
}
class Bullet {
private:
	Direction bulletDirec;//子弹的方向
	int bullet_x;
	int bullet_y;
	bool ifShoot = 0;
	int bullet_state = 0;	//0-未开火，1-刚开火，2-子弹在飞，3-子弹碰到刚体
	int boom_x, boom_y;		//record the boom place
	/*boom 有关数据*/
	int show = 0;				//当show为2的时候才会执行爆炸贴图
	int tick = 0;				//used for counting time
	int speed = unit * 7 / 10;
	IMAGE bullet[4], bullet_hide[4];
	int i1, i2, j1, j2;
public:
	Bullet(int speed = unit * 7 / 10);
	void setBulletX(int);
	void setBulletY(int);
	int getBulletX()const;
	int getBulletY()const;
	bool IfShoot()const;
	void setIfShoot(bool);
	int getBulletState()const;
	void setBulletDir(Direction);
	void setBulletState(int);
	Direction getBulletDir()const;
	Bullet loadBullet();
	void showBullet();
	void shoot(int x, int y, Direction tankDir);
	void bulletMove();
	void loadBoom();
	void showBoom();
	void setShow(int);
};
Bullet::Bullet(int s)
	:speed(s), show(0), tick(0)
{
}
void Bullet::setBulletX(int a)
{
	bullet_x = a;
}
void Bullet::setBulletY(int a)
{
	bullet_y = a;
}
int Bullet::getBulletX() const
{
	return bullet_x;
}
int Bullet::getBulletY() const
{
	return bullet_y;
}
bool Bullet::IfShoot() const
{
	return ifShoot;
}
void Bullet::setIfShoot(bool a)
{
	ifShoot = a;
}
int Bullet::getBulletState() const
{
	return bullet_state;
}
void Bullet::setBulletDir(Direction a)
{
	bulletDirec = a;
}
void Bullet::setBulletState(int a)
{
	bullet_state = a;
}
Direction Bullet::getBulletDir() const
{
	return bulletDirec;
}
Bullet Bullet::loadBullet()
{
	char bulletfile[34] = "";
	for (int i = 0; i < 4; i++) {
		sprintf_s(bulletfile, sizeof(bulletfile), ".\\image\\bullet\\%d.png", i);
		loadimage(bullet + i, bulletfile, unit / 2, unit / 2);
		sprintf_s(bulletfile, sizeof(bulletfile), ".\\image\\bullet\\%dh.png", i);
		loadimage(bullet_hide + i, bulletfile, unit / 2, unit / 2);
	}
	mciSendString("open .\\Audio\\Shoot.mp3 alias shoot1", 0, 0, 0);
	mciSendString("open .\\Audio\\Shoot.mp3 alias shoot2", 0, 0, 0);
	mciSendString("open .\\Audio\\Shoot.mp3 alias shoot3", 0, 0, 0);
	mciSendString("open .\\Audio\\Shoot.mp3 alias shoot4", 0, 0, 0);
	return *this;
}
void Bullet::showBullet()
{
	putimage(getBulletX(), getBulletY(), bullet_hide + getBulletDir(), SRCAND);
	putimage(getBulletX(), getBulletY(), bullet + getBulletDir(), SRCPAINT);
}
void Bullet::shoot(int x, int y, Direction tankDir)
{
	playShootSound();
	setBulletDir(tankDir);
	switch (getBulletDir())
	{
	case UP:
		setBulletX(x + unit / 4 * 3);
		setBulletY(y - unit / 2);
		putimage(getBulletX(), getBulletY(), bullet_hide, SRCAND);
		putimage(getBulletX(), getBulletY(), bullet, SRCPAINT);
		break;
	case DOWN:
		setBulletX(x + unit / 4 * 3);
		setBulletY(y + unit * 2);
		putimage(getBulletX(), getBulletY(), bullet_hide + 1, SRCAND);
		putimage(getBulletX(), getBulletY(), bullet + 1, SRCPAINT);
		break;
	case LEFT:
		setBulletX(x - unit / 2);
		setBulletY(y + unit / 4 * 3);
		putimage(getBulletX(), getBulletY(), bullet_hide + 2, SRCAND);
		putimage(getBulletX(), getBulletY(), bullet + 2, SRCPAINT);
		break;
	case RIGHT:
		setBulletX(x + unit * 2);
		setBulletY(y + unit / 4 * 3);
		putimage(getBulletX(), getBulletY(), bullet_hide + 3, SRCAND);
		putimage(getBulletX(), getBulletY(), bullet + 3, SRCPAINT);
		break;
	default:
		break;
	}
}
void Bullet::bulletMove()
{
	/*int i1, i2, j1, j2;*/
	//border detection:是否打到了边界
	if (bullet_x <= 0 || bullet_x >= unit * map_x || bullet_y <= 0 || bullet_y >= unit * map_y) {
		bullet_state = 0;
		boom_x = bullet_x + unit / 4 - 2 * unit;
		boom_y = bullet_y - 2 * unit;
	}
	else {
		//加载子弹的图片
		switch (bulletDirec)
		{
		case UP:
			j1 = bullet_x / unit;
			j2 = (bullet_x + unit / 2) / unit;
			if (bullet_y % unit == 0 && bullet_y != 0) {
				i1 = bullet_y / unit - 1;
			}
			else {
				i1 = bullet_y / unit;
			}
			//detection:
			if (j2 < map_x) {//防止溢出
				if (((map[i1][j1] == Wall) || (map[i1][j1] == Iron)) || ((map[i1][j2] == Wall) || (map[i1][j2] == Iron))) {
					boom_x = bullet_x + unit / 4 - 2 * unit;				//boom place
					boom_y = bullet_y - 2 * unit;
					bullet_state = 0;
					if (map[i1][j1] == Wall) {
						map[i1][j1] = 0;
					}
					if (map[i1][j2] == Wall) {
						map[i1][j2] = 0;
					}
				}
				else {
					bullet_y -= speed;
				}
			}
			else {
				if ((map[i1][j1] == Wall) || (map[i1][j1] == Iron)) {
					boom_x = bullet_x + unit / 4 - 2 * unit;				//boom place
					boom_y = bullet_y - 2 * unit;
					bullet_state = 0;
					if (map[i1][j1] == Wall) {
						map[i1][j1] = 0;
					}
				}
				else {
					bullet_y -= speed;
				}
			}
			break;
		case DOWN:
			j1 = bullet_x / unit;
			j2 = (bullet_x + unit / 2) / unit;
			i1 = (bullet_y + unit / 2) / unit;
			if (i1 == 26)i1 = 25;
			//detection:
			if (j2 < map_x) {//防止溢出
				if (map[i1][j1] == Wall || map[i1][j1] == Iron || map[i1][j2] == Wall || map[i1][j2] == Iron || map[i1][j2] == Home || map[i1][j2] == 4 || map[i1][j1] == Home || map[i1][j1] == 4) {
					boom_x = bullet_x + unit / 4 - 2 * unit;
					boom_y = bullet_y - 2 * unit;
					bullet_state = 0;
					if (map[i1][j1] == Wall) {
						map[i1][j1] = 0;
					}
					if (map[i1][j2] == Wall) {
						map[i1][j2] = 0;
					}
					if (map[i1][j1] == Home || map[i1][j1] == 4 || map[i1][j2] == Home || map[i1][j2] == 4) {
						map[24][12] = DestroyHome;
						gameover = 1;
					}
				}
				else {
					bullet_y += speed;
				}
			}
			else {
				if ((map[i1][j1] == Wall) || (map[i1][j1] == Iron) || map[i1][j1] == Home || map[i1][j1] == 4) {
					boom_x = bullet_x + unit / 4 - 2 * unit;				//boom place
					boom_y = bullet_y - 2 * unit;
					bullet_state = 0;
					if (map[i1][j1] == Wall) {
						map[i1][j1] = 0;
					}
					if (map[i1][j1] == Home || map[i1][j1] == 4) {
						map[24][12] = DestroyHome;
						gameover = 1;
					}
				}
				else {
					bullet_y += speed;
				}
			}
			break;
		case LEFT:
			i1 = bullet_y / unit;
			i2 = (bullet_y + unit / 2) / unit;
			if ((bullet_x / unit) % unit == 0) {
				j1 = bullet_x / unit - 1;
			}
			else {
				j1 = bullet_x / unit;
			}
			if (i2 < map_y) {
				if (map[i1][j1] == Wall || map[i2][j1] == Wall || map[i1][j1] == Iron || map[i2][j1] == Iron || map[i1][j1] == Home || map[i2][j1] == Home || map[i1][j1] == 4 || map[i2][j1] == 4) {
					boom_x = bullet_x + unit / 4 - 2 * unit;				//boom place
					boom_y = bullet_y - 2 * unit;
					bullet_state = 0;
					if (map[i1][j1] == Wall) {
						map[i1][j1] = 0;
					}
					if (map[i2][j1] == Wall) {
						map[i2][j1] = 0;
					}
					if (map[i1][j1] == Home || map[i1][j1] == 4 || map[i2][j1] == Home || map[i2][j1] == 4) {
						map[24][12] = DestroyHome;
						gameover = 1;
					}
				}
				else
				{
					bullet_x -= speed;
				}
			}
			else
			{
				if (map[i1][j1] == Wall || map[i1][j1] == Iron || map[i1][j1] == Home || map[i1][j1] == 4) {
					boom_x = bullet_x + unit / 4 - 2 * unit;				//boom place
					boom_y = bullet_y - 2 * unit;
					bullet_state = 0;
					if (map[i1][j1] == Wall) {
						map[i1][j1] = 0;
					}
					if (map[i1][j1] == Home || map[i1][j1] == 4) {
						map[24][12] = DestroyHome;
						gameover = 1;
					}
				}
				else
				{
					bullet_x -= speed;
				}
			}
			break;
		case RIGHT:
			i1 = bullet_y / unit;
			i2 = (bullet_y + unit / 2) / unit;
			j1 = (bullet_x + unit / 2) / unit;
			if (j1 == map_x) {
				boom_x = bullet_x + unit / 4 - 2 * unit;				//boom place
				boom_y = bullet_y - 2 * unit;
				bullet_state = 0;
			}
			else if (i2 < map_y) {
				if (map[i1][j1] == Wall || map[i1][j1] == Iron || map[i2][j1] == Wall || map[i2][j1] == Iron || map[i1][j1] == Home || map[i1][j1] == 4 || map[i2][j1] == Home || map[i2][j1] == 4) {
					boom_x = bullet_x + unit / 4 - 2 * unit;				//boom place
					boom_y = bullet_y - 2 * unit;
					bullet_state = 0;
					if (map[i1][j1] == Wall) {
						map[i1][j1] = 0;
					}
					if (map[i2][j1] == Wall) {
						map[i2][j1] = 0;
					}
					if (map[i1][j1] == Home || map[i1][j1] == 4 || map[i2][j1] == Home || map[i2][j1] == 4) {
						map[24][12] = DestroyHome;
						gameover = 1;
					}
				}
				else
				{
					bullet_x += speed;
				}
			}
			else
			{
				if (map[i1][j1] == Wall || map[i1][j1] == Iron || map[i1][j1] == Home || map[i1][j1] == 4) {
					boom_x = bullet_x + unit / 4 - 2 * unit;				//boom place
					boom_y = bullet_y - 2 * unit;
					bullet_state = 0;
					if (map[i1][j1] == Wall) {
						map[i1][j1] = 0;
					}
					if (map[i1][j1] == Home || map[i1][j1] == 4) {
						map[24][12] = DestroyHome;
						gameover = 1;
					}
				}
				else
				{
					bullet_x += speed;
				}
			}
			break;
		default:
			break;
		}
	}
}
void Bullet::loadBoom()
{
	char boomfile[34] = "";
	for (int i = 0; i < 4; i++) {
		sprintf_s(boomfile, sizeof(boomfile), ".\\image\\boom\\%d.png", i);
		loadimage(boom + i, boomfile, unit * 4, unit * 4);
		sprintf_s(boomfile, sizeof(boomfile), ".\\image\\boom\\%dh.png", i);
		loadimage(boom_hide + i, boomfile, unit * 4, unit * 4);
	}
	mciSendString("open ..\\tank_game\\Audio\\boom_wall.mp3 alias destroywall", 0, 0, 0);
}
void Bullet::showBoom()
{

	if (show == 2) {
		putimage(boom_x, boom_y, boom_hide + tick / 4, SRCAND);
		putimage(boom_x, boom_y, boom + tick / 4, SRCPAINT);
		tick++;
		if (tick % 4 == 0 && tick <= 16) {
			//counter++;
			show = 2;
		}
		if (tick == 20) {
			show = 0;						//finish the boom effect
			tick = 0;
			//counter = 0;
		}
	}
	else if (bullet_state == 1) {			//在由1变为0的那一刻起短暂执行！
		show = 1;
	}
	if (bullet_state == 0 && show == 1) {
		show = 2;
	}
}
void Bullet::setShow(int a)
{
	show = a;
}

//*************Tank*****************
class Tank {
private:
	int num_w = 0;
	int num_s = 2;
	int num_a = 4;
	int num_d = 6;
	int speed = unit / 10;
	int HP = 1;
	int shootSpeed = unit * 7 / 10;
	Direction curDir;
	int tank_x;
	int tank_y;
	int birthplace_x;
	int birthplace_y;
	int shootState = 0;
	bool borndone = 0;	//出生的闪光特效是否结束	
	int counter;		//用于出生特效的计数
protected:
	Bullet bl;
public:
	Tank(Direction CurDirection = UP, int birX = 0, int birY = 0, int Speed = unit / 10, int hp = 1, int Shootspeed = unit * 7 / 10, int tankX = 0, int tankY = 0);
	/***************
		set类函数
	****************/
	void setNumW(int);
	void setNumS(int);
	void setNumA(int);
	void setNumD(int);
	void setSpeed(int);
	void setHP(int);
	void setShootSpeed(int);
	void setCurDir(Direction);
	void setTankX(int);
	void setTankY(int);
	void setBirthX(int);
	void setBirthY(int);
	void setShootState(int);
	void setBulletState(int);
	void setBulletDir(Direction);
	void setBornDone(bool);
	void setCounter(int);
	/***************
		get类函数
	****************/
	int getNumA()const;
	int getNumS()const;
	int getNumW()const;
	int getNumD()const;
	int getSpeed()const;
	int getHP()const;
	int getShootSpeed()const;
	int getShootState()const;
	int getTankX()const;
	int getTankY()const;
	int getBirthX()const;
	int getBirthY()const;
	Direction getCurDir()const;
	Direction getBulletDir()const;
	int getBulletState()const;
	bool getBornDone()const;
	int getCounter()const;
	/*************
		功能函数
	**************/
	//切换关卡时可能会出现继续上一关的爆炸特效，需要清理
	void clearBoom();
	void loadBullet();
	void bornEffect();
	bool movable(Direction, int x, int y);
	void move(Direction);
	virtual void show() = 0;
	virtual void loadTank() = 0;
	//	virtual void shoot();
	virtual void showBullet();
	void shoot();//初始化子弹的位置
	void showBoom();
};
Tank::Tank(Direction CurDirection, int birX, int birY, int Speed, int hp, int Shootspeed, int tankX, int tankY)
	:num_w(0), num_s(2), num_a(4), num_d(6), speed(Speed), HP(hp), shootSpeed(Shootspeed), curDir(CurDirection), tank_x(birX), tank_y(birY), birthplace_x(birX), birthplace_y(birY),shootState(0)
{
	bl.setBulletState(0);
}
void Tank::setNumW(int a)
{
	num_w = a;
}
void Tank::setNumS(int a)
{
	num_s = a;
}
void Tank::setNumA(int a)
{
	num_a = a;
}
void Tank::setNumD(int a)
{
	num_d = a;
}
void Tank::setSpeed(int s)
{
	speed = s;
}
void Tank::setHP(int a)
{
	HP = a;
}
void Tank::setShootSpeed(int a)
{
	shootSpeed = a;
}
void Tank::setCurDir(Direction a)
{
	curDir = a;
}
void Tank::setTankX(int a)
{
	tank_x = a;
}
void Tank::setTankY(int y)
{
	tank_y = y;
}
void Tank::setBirthX(int x)
{
	birthplace_x = x;
}
void Tank::setBirthY(int y)
{
	birthplace_y = y;
}
void Tank::setShootState(int a)
{
	shootState = a;
}
void Tank::setBulletState(int a)
{
	bl.setBulletState(a);
}
void Tank::setBulletDir(Direction d)
{
	bl.setBulletDir(d);
}
void Tank::setBornDone(bool a)
{
	borndone = a;
}
void Tank::setCounter(int a)
{
	counter = a;
}
int Tank::getNumA() const
{
	return num_a;
}
int Tank::getNumS() const
{
	return num_s;
}
int Tank::getNumW() const
{
	return num_w;
}
int Tank::getNumD() const
{
	return num_d;
}
int Tank::getSpeed() const
{
	return speed;
}
int Tank::getHP() const
{
	return HP;
}
int Tank::getShootSpeed() const
{
	return shootSpeed;
}
int Tank::getShootState() const
{
	return shootState;
}
int Tank::getTankX() const
{
	return tank_x;
}
int Tank::getTankY() const
{
	return tank_y;
}
int Tank::getBirthX() const
{
	return birthplace_x;
}
int Tank::getBirthY() const
{
	return birthplace_y;
}
Direction Tank::getCurDir() const
{
	return curDir;
}
Direction Tank::getBulletDir() const
{
	return bl.getBulletDir();
}
int Tank::getBulletState() const
{
	return bl.getBulletState();
}
bool Tank::getBornDone() const
{
	return borndone;
}
int Tank::getCounter() const
{
	return counter;
}
void Tank::clearBoom()
{
	bl.setShow(0);
}
void Tank::loadBullet()
{
	bl.loadBullet();
}
void Tank::bornEffect()
{
	if (borndone == 0&&counter!=16) {
		putimage(birthplace_x, birthplace_y, star_hide + counter / 4, SRCAND);
		putimage(birthplace_x, birthplace_y, star + counter / 4, SRCPAINT);
	}else if (counter == 16) {
		putimage(birthplace_x, birthplace_y, star_hide + 3, SRCAND);
		putimage(birthplace_x, birthplace_y, star + 3, SRCPAINT);
	}
}
bool Tank::movable(Direction dir, int x, int y)
{
	int i, j;
	switch (dir) {
	case UP:
		i = y / unit - 1;
		j = x / unit;
		if (y % unit != 0) {//校准
			i++;
		}
		if (y == 0)return false;//地图上边界
		else if (x % unit >= speed * 3) {//自动矫正
			if ((map[i][j] == Jungle || map[i][j] == Empty || map[i][j] == Ice) && (map[i][j + 1] == Jungle || map[i][j + 1] == Empty || map[i][j + 1] == Ice) && (map[i][j + 2] == Jungle || map[i][j + 2] == Empty || map[i][j + 2] == Ice))
			{
				return true;
			}
			else return false;
		}
		else {
			if ((map[i][j] == Jungle || map[i][j] == Empty || map[i][j] == Ice) && (map[i][j + 1] == Jungle || map[i][j + 1] == Empty || map[i][j + 1] == Ice))
			{
				return true;
			}
			else return false;
		}
		break;
	case DOWN:
		i = y / unit + 2;
		j = x / unit;
		if (y >= (map_y - 2) * unit)return false;//地图下边界
		else if (x % unit >= speed * 2) {//矫正
			if ((map[i][j] == Jungle || map[i][j] == Empty || map[i][j] == Ice) && (map[i][j + 1] == Jungle || map[i][j + 1] == Empty || map[i][j + 1] == Ice) && (map[i][j + 2] == Jungle || map[i][j + 2] == Empty || map[i][j + 2] == Ice))
			{
				return true;
			}
			else return false;
		}
		else {
			if ((map[i][j] == Jungle || map[i][j] == Empty || map[i][j] == Ice) && (map[i][j + 1] == Jungle || map[i][j + 1] == Empty || map[i][j + 1] == Ice))
			{
				return true;
			}
			else return false;
		}
		break;
	case LEFT:
		i = y / unit;
		j = x / unit - 1;//
		if (x % unit != 0)
		{
			j++;
		}
		if (x == 0)return false;//地图左边界
		else if (y % unit >= speed * 3) {
			if ((map[i][j] == Jungle || map[i][j] == Empty || map[i][j] == Ice) && (map[i + 1][j] == Jungle || map[i + 1][j] == Empty || map[i + 1][j] == Ice) && (map[i + 2][j] == Jungle || map[i + 2][j] == Empty || map[i + 2][j] == Ice))
			{
				return true;
			}
			else return false;
		}
		else
		{
			if ((map[i][j] == Jungle || map[i][j] == Empty || map[i][j] == Ice) && (map[i + 1][j] == Jungle || map[i + 1][j] == Empty || map[i + 1][j] == Ice))
			{
				return true;
			}
			else return false;
		}
		break;
	case RIGHT:
		i = y / unit;
		j = x / unit + 2;
		if (x >= (map_x - 2) * unit)return false;
		else if (y % unit >= speed * 2) {
			if ((map[i][j] == Jungle || map[i][j] == Empty || map[i][j] == Ice) && (map[i + 1][j] == Jungle || map[i + 1][j] == Empty || map[i + 1][j] == Ice) && (map[i + 2][j] == Jungle || map[i + 2][j] == Empty || map[i + 2][j] == Ice))
			{
				return true;
			}
			else return false;
		}
		else
		{
			if ((map[i][j] == Jungle || map[i][j] == Empty || map[i][j] == Ice) && (map[i + 1][j] == Jungle || map[i + 1][j] == Empty || map[i + 1][j] == Ice))
			{
				return true;
			}
			else return false;
		}
		break;
	default:
		return false;
		break;
	}
}
void Tank::move(Direction tmp)
{
	if (movable(tmp, tank_x, tank_y)) {
		switch (tmp) {
		case UP:
			if ((tank_x % unit <= speed * 3 || tank_x % unit >= unit - speed * 3)) {//矫正方位
				if (tank_x % unit <= speed * 3) {
					tank_x -= tank_x % unit;
				}
				else {
					tank_x += unit - (tank_x % unit);
				}
			}
			/*	if(tank_x)*///冰面待修正
			if (map[tank_y / unit][tank_x / unit] == Ice) {
				tank_y -= speed * 2;
			}
			tank_y -= speed;//先变值再加载
			//冰面的步伐更大
			num_w++;
			if (num_w == 2)num_w = 0;
			setCurDir(UP);
			break;
		case DOWN:
			if ((tank_x % unit <= speed * 3 || tank_x % unit >= unit - speed * 3)) {
				if (tank_x % unit <= speed * 3) {
					tank_x -= tank_x % unit;
				}
				else {
					tank_x += unit - (tank_x % unit);
				}
			}
			if (map[tank_y / unit + 2][tank_x / unit] == Ice) {
				tank_y += speed * 2;
			}
			tank_y += speed;
			num_s++;
			if (num_s == 4)num_s = 2;
			setCurDir(DOWN);
			break;
		case LEFT:
			if ((tank_y % unit <= speed * 3 || tank_y % unit >= unit - speed * 3)) {
				if (tank_y % unit <= speed * 3) {
					tank_y -= tank_y % unit;
				}
				else {
					tank_y += unit - (tank_y % unit);
				}
			}
			if (map[tank_y / unit][tank_x / unit] == Ice) {
				tank_x -= speed * 2;
			}
			tank_x -= speed;
			num_a++;
			if (num_a == 6)num_a = 4;
			setCurDir(LEFT);
			break;
		case RIGHT:
			if ((tank_y % unit <= speed * 3 || tank_y % unit >= unit - speed * 3)) {
				if (tank_y % unit <= speed * 3) {
					tank_y -= tank_y % unit;
				}
				else {
					tank_y += unit - (tank_y % unit);
				}
			}
			if (map[tank_y / unit + 2][tank_x / unit] == Ice) {
				tank_x += speed * 2;
			}
			tank_x += speed;
			num_d++;
			if (num_d == 8)num_d = 6;
			setCurDir(RIGHT);
			break;
		default:
			break;
		}
	}
	//cannot move but show the effect
	else {
		switch (tmp) {
		case UP:			//没有引号
			num_w++;
			setCurDir(UP);
			if (num_w == 2)num_w = 0;
			break;
		case DOWN:
			num_s++;
			setCurDir(DOWN);
			if (num_s == 4)num_s = 2;
			break;
		case LEFT:
			num_a++;
			setCurDir(LEFT);
			if (num_a == 6)num_a = 4;
			break;
		case RIGHT:
			num_d++;
			setCurDir(RIGHT);
			if (num_d == 8)num_d = 6;
			break;
		default:
			break;
		}
	}
}
void Tank::showBullet()
{
	if (shootState == 1 && getBulletState() == 1) {
		bl.showBullet();
		bl.bulletMove();
	}
}
void Tank::shoot()
{
	setShootState(1);
	setBulletState(1);
	setBulletDir(getCurDir());
	bl.shoot(getTankX(), getTankY(), getCurDir());
}
void Tank::showBoom()
{
	bl.loadBoom();
	bl.showBoom();
}

//********enemyTank***********
class enemyTank :public Tank
{
private:
	enemyType type = enemyType::normal;
	int counter = 0;
	int tick = 0;
	bool borndone = 0;
	int seed;//use for finding directions
	Direction lastDirection = 1;//former direction
	Bullet bl;
	int shootState = 0;//开火状态
	IMAGE EnemyTank[32], EnemyTank_hide[16];
public:
	enemyTank(enemyType = enemyType::normal);
	//~enemyTank();
	/*****set类函数*****/
	void setBornDone(bool);
	void setType(enemyType);
	void setSeed(int);
	/*******get类函数*******/
	int getSeed()const;
	enemyType getType()const;
	bool getBorndone()const;
	//load
	void loadTank()override;
	/******show*******/
	void show()override;
	Direction findDir(int seed);
	void born();
	void OShoot();
};
enemyType enemyTank::getType() const
{
	return type;
}
void enemyTank::loadTank()
{//hide
	for (int type = 0, dir = 0, i = 0; i < 16; i++) {
		char enemy_hide_file[45] = "";
		sprintf_s(enemy_hide_file, sizeof(enemy_hide_file), ".\\image\\tank\\enemy_hide\\%d%d.png", type, dir);
		loadimage(EnemyTank_hide + i, enemy_hide_file, unit * 2, unit * 2);
		dir++;
		if ((i + 1) % 4 == 0 && i != 0) {
			type++;
			dir = 0;
		}

	}
	//load enemy tank
	for (int type = 0, p = 1, dir = 0, i = 0; i < 32; i++) {//direction 0123,picture 1/2
		char enemyfile[40] = "";
		sprintf_s(enemyfile, sizeof(enemyfile), ".\\Image\\tank\\enemy\\%d%d%d.png", type, dir, p);
		loadimage(EnemyTank + i, enemyfile, unit * 2, unit * 2);
		p++;
		if (p == 3) {
			dir++;
			p = 1;
		}
		if ((i + 1) % 8 == 0 && i != 0)type++;
		if (dir == 4) {
			dir = 0;
		}
	}
}
void enemyTank::show()
{
	switch (getCurDir())
	{
		putimage(getTankX(), getTankY(), EnemyTank_hide + getCurDir() + 4 * int(type), SRCAND);
	case UP:
		putimage(getTankX(), getTankY(), EnemyTank + getNumW() + 8 * int(type), SRCPAINT);
		break;
	case DOWN:
		putimage(getTankX(), getTankY(), EnemyTank + getNumS() + 8 * int(type), SRCPAINT);
		break;
	case LEFT:
		putimage(getTankX(), getTankY(), EnemyTank + getNumA() + 8 * int(type), SRCPAINT);
		break;
	case RIGHT:
		putimage(getTankX(), getTankY(), EnemyTank + getNumD() + 8 * int(type), SRCPAINT);
		break;
	default:
		break;
	}
}
Direction enemyTank::findDir(int seed)
{
	//record the former direction
	counter++;
	srand(seed);
	int r = rand() % 100;
	if (unit * 24 < getTankY())
	{
		if (unit * 9 < getTankX())
		{
			if (r < 50 && movable(UP, getTankX(), getTankY())) {
				lastDirection = UP;
				return UP;
			}
			else {
				lastDirection = LEFT;
				return LEFT;
			}
		}
		else if (unit * 9 > getTankX())
		{
			if (r < 50 && movable(RIGHT, getTankX(), getTankY())) {
				lastDirection = RIGHT;
				return RIGHT;
			}
			else {
				lastDirection = UP;
				return UP;
			}
		}
		else if (unit * 9 == getTankX())
		{
			if (map[getTankY() / unit - 1][getTankX() / unit] == Iron)
			{
				if (r < 50) {
					lastDirection = LEFT;
					return LEFT;
				}
				else {
					lastDirection = RIGHT;
					return RIGHT;
				}
			}
			lastDirection = UP;
			return UP;
		}
	}
	else if (unit * 24 > getTankY())
	{
		if (unit * 9 < getTankX())
		{
			if (r < 50 && movable(DOWN, getTankX(), getTankY())) {
				lastDirection = DOWN;
				return DOWN;
			}
			else {
				lastDirection = LEFT;
				return LEFT;
			}
		}
		else if (unit * 9 > getTankX())
		{
			if (r < 50 && movable(RIGHT, getTankX(), getTankY())) {
				lastDirection = RIGHT;
				return RIGHT;
			}
			else {
				lastDirection = DOWN;
				return DOWN;
			}
		}
		else if (unit * 9 == getTankX())
		{
			if (map[getTankY() / unit + 2][getTankX() / unit] == Iron)
			{
				if (r < 50) {
					lastDirection = LEFT;
					return LEFT;
				}
				else {
					lastDirection = RIGHT;
					return RIGHT;
				}
			}
			lastDirection = DOWN;
			return DOWN;
		}
	}
	else if (unit * 24 == getTankY())
	{
		if (unit * 9 < getTankX()) {
			lastDirection = LEFT;
			return LEFT;
		}
		else {
			lastDirection = RIGHT;
			return RIGHT;
		}
	}
	else {
		return lastDirection;
	}
}
void enemyTank::born()
{
	//setRandom birthplace
	setBirthX(rand() % 25);
	int tmp = rand() % 4;
	while (!(map[tmp][getBirthX()] == 0 && map[tmp][getBirthX() + 1] == 0 && map[tmp + 1][getBirthX()] == 0 && map[tmp + 1][getBirthX() + 1] == 0)) {
		setBirthX(getBirthX() + 1);
		if (getBirthX() >= 24) {
			setBirthX(rand() % 25);
			tmp = rand() % 4;
		}
	}
	setBirthX(getBirthX() * unit);
	setBirthY(tmp * unit);
	setTankX(getBirthX());
	setTankY(getBirthY());
}
void enemyTank::OShoot()
{
	bl.loadBullet();	
}
enemyTank::enemyTank(enemyType t)
	:Tank(DOWN)
{
	born();
	setType(t);
	if (t == enemyType::shootFaster) {
		setShootSpeed(unit * 10);
	}
	if (t == enemyType::moveFaster) {
		setSpeed(unit / 5);
	}
	if (t == enemyType::stronger) {
		setHP(2);
	}
}
void enemyTank::setType(enemyType a)
{
	type = a;
}
void enemyTank::setSeed(int a)
{
	seed = a;
}
void enemyTank::setBornDone(bool a)
{
	borndone = a;
}
bool enemyTank::getBorndone() const
{
	return borndone;
}
int enemyTank::getSeed() const
{
	return seed;
}

//********user*************
class UserTank :public Tank
{
private:
	int score=0;
	IMAGE user1Tank_hide[16], user1Tank[32];
	int level=0;						//玩家坦克的等级
public:
	UserTank(int bx, int by,int lv=0);
	void setScore(int);
	int getScore()const;
	void setLevel(int);
	int getLevel()const;
	/*show user's tank*/
	virtual void show()override;
	virtual void loadTank()override;
	/*****
	bullet
	******/
	void  loadBullet();
	void setBulletDir(Direction);
	void setBulletState(int);
	Direction getBulletDir()const;
	int getBulletState()const;
};
void UserTank::setScore(int a)
{
	score = 0;
}
int UserTank::getScore() const
{
	return score;
}
void UserTank::setLevel(int a)
{
	level = a;
}
int UserTank::getLevel() const
{
	return level;
}
UserTank::UserTank(int bx, int by,int lv)
	:Tank(UP, bx, by), score(0),level(lv)
{
	if (level == 1) {
		setSpeed(unit / 7);
	}
	else if (level == 2) {
		setSpeed(unit / 7);
		setShootSpeed(unit / 8 * 10);
	}
	else if (level == 3) {
		setSpeed(unit / 8);
		setShootSpeed(unit / 9 * 10);
	}
}
void UserTank::show()
{
	switch (getCurDir())
	{
		putimage(getTankX(), getTankY(), user1Tank_hide + getCurDir() + 4 * level, SRCAND);
	case UP:
		putimage(getTankX(), getTankY(), user1Tank + getNumW() + 8 * level, SRCPAINT);
		break;
	case DOWN:
		putimage(getTankX(), getTankY(), user1Tank + getNumS() + 8 * level, SRCPAINT);
		break;
	case LEFT:
		putimage(getTankX(), getTankY(), user1Tank + getNumA() + 8 * level, SRCPAINT);
		break;
	case RIGHT:
		putimage(getTankX(), getTankY(), user1Tank + getNumD() + 8 * level, SRCPAINT);
		break;
	default:
		break;
	}
}
void UserTank::loadTank()
{
	for (int tankLevel = 0, dir = 0, i = 0; i < 16; i++) {
		char usertank_hide_file[43] = "";
		sprintf_s(usertank_hide_file, sizeof(usertank_hide_file), ".\\image\\tank\\player_hide\\%d%d.png", tankLevel, dir);
		loadimage(&(user1Tank_hide[i]), usertank_hide_file, unit * 2, unit * 2);
		dir++;
		if ((i + 1) % 4 == 0 && i != 0) {
			tankLevel++;
			dir = 0;
		}

	}
	for (int tankLevel = 0, p = 1, dir = 0, i = 0; i < 32; i++) {//direction 0123,picture 1/2
		char tankfile[41] = "";
		sprintf_s(tankfile, sizeof(tankfile), ".\\Image\\tank\\player1\\%d%d%d.png", tankLevel, dir, p);
		loadimage(user1Tank + i, tankfile, unit * 2, unit * 2);
		p++;
		if (p == 3) {
			dir++;
			p = 1;
		}
		if ((i + 1) % 8 == 0 && i != 0)tankLevel++;
		if (dir == 4) {
			dir = 0;
		}
	}
}
void UserTank::loadBullet()
{
	bl.loadBullet();
}
void UserTank::setBulletDir(Direction a)
{
	bl.setBulletDir(a);
}
void UserTank::setBulletState(int s)
{
	bl.setBulletState(s);
}
Direction UserTank::getBulletDir() const
{
	return bl.getBulletDir();
}
int UserTank::getBulletState() const
{
	return bl.getBulletState();
}

enemyTank* eTank=new enemyTank[enemyNumber];
UserTank u1(9 * unit, 24 * unit);//user1
UserTank u2(15 * unit, 24 * unit,2);//user2
enemyTank tank(enemyType::moveFaster);
enemyTank tank2(enemyType::stronger);

//***********resource***************
void playbkmusic() {
	mciSendString("open .\\Audio\\GameBegin.mp3 alias bkmusic", 0, 0, 0);
	mciSendString("play bkmusic repeat", 0, 0, 0);
}
void closebkmusic() {
	mciSendString("close bkmusic", 0, 0, 0);
}
void loadGameMenu() {
	loadimage(&gameMenuBackground, ".\\image\\greybg.png", unit * 10, unit * 26);
	loadimage(&enemyMark, ".\\image\\mark\\enemy_mark.png", unit, unit);
	loadimage(&enemyMark_hide, ".\\image\\mark\\enemy_mark_hide.png", unit, unit);
	loadimage(&playerMark1, ".\\image\\mark\\player_mark.png", unit, unit);
	loadimage(&playerMark2, ".\\image\\mark\\player_mark.png", unit, unit);
	loadimage(&playerMark1_hide, ".\\image\\mark\\player_mark_hide.png", unit, unit);
	loadimage(&playerMark2_hide, ".\\image\\mark\\player_mark_hide.png", unit, unit);
	loadimage(&flag, ".\\image\\flag.png", unit * 2, unit * 2);
	loadimage(&flag_hide, ".\\image\\flag_hide.png", unit * 2, unit * 2);
}
void loadBornStar() {
	char starfile[34] = "";
	for (int i = 0; i < 4; i++) {
		sprintf_s(starfile, sizeof(starfile), ".\\Image\\star\\%d.png", i);
		loadimage(star + i, starfile, unit * 2, unit * 2);
		sprintf_s(starfile, sizeof(starfile), ".\\Image\\star\\%dh.png", i);
		loadimage(star_hide + i, starfile, unit * 2, unit * 2);
	}
}
void loadMap() {
	//加载地图素材：
	loadimage(&wall, ".\\image\\map\\wall.png", unit, unit);
	loadimage(&iron, ".\\image\\map\\iron.png", unit, unit);
	loadimage(&ice, ".\\image\\map\\ice.png", unit, unit);
	loadimage(home, ".\\image\\commander0.png", unit * 2, unit * 2);
	loadimage(home + 1, ".\\image\\commander1.png", unit * 2, unit * 2);
	loadimage(home_hide, ".\\image\\commander0_hide.png", unit * 2, unit * 2);
	loadimage(home_hide + 1, ".\\image\\commander1_hide.png", unit * 2, unit * 2);
	loadimage(&jungle, ".\\image\\map\\jungle.png", unit, unit);
	loadimage(&jungle_hide, ".\\image\\map\\jungle_hide.png", unit, unit);
	loadimage(sea, ".\\image\\map\\sea0.png", unit, unit);
	loadimage(sea + 1, ".\\image\\map\\sea1.png", unit, unit);
	loadimage(&border, ".\\image\\map\\border.png", unit, unit);
	//加载地图背景：
	loadimage(&background, "blackbkg.jpg", 32 * unit, 30 * unit);
	/*从文件当中加载地图数组*/
	ifstream mapfile("Map.txt", ios::in);
	if (!mapfile) {
		cout << "The map file can not be open!" << endl;
		exit(0);
	}
	mapfile.seekg(2 * (stage - 1) * map_x * map_y, ios::beg);
	for (int i = 0; i < map_x; i++) {
		for (int j = 0; j < map_y; j++) {
			mapfile >> map[i][j];
		}
	}
	mapfile.close();
}
void putjungle() {
	for (int i = 0; i < map_x; i++) {
		for (int j = 0; j < map_y; j++) {
			switch (map[i][j])
			{
			case 5:putimage(j * unit, i * unit, &jungle_hide, SRCAND);
				putimage(j * unit, i * unit, &jungle, SRCPAINT);
				break;
			default:
				break;
			}
		}
	}
}
void loadGameResource() {
	u1.loadTank();
	u2.loadTank();
	u1.loadBullet();
	u2.loadBullet();
	loadBornStar();
	loadMap();
	loadGameMenu();
}

//**************Menu***************
void ChangeLanguage()
{
	keybd_event(VK_SHIFT, 0, 0, 0);
	Sleep(100);
	keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
}
void drawMenu(int width, int height, int bkc)
{
	int clickTime = 0;
	//初始化窗口：
	initgraph(width, height);
	//设置背景颜色
	setbkcolor(bkc);
	cleardevice();
	//设置字体的样式：
	setbkmode(TRANSPARENT);
	settextcolor(BRICK);
	settextstyle(width * 0.092, 0, "方正粗黑宋简体");
	outtextxy(width * 0.3, height * 0.24, "坦  克  大  战");
	//画矩形：
	setfillcolor(LIGHTBLUE);
	fillroundrect(0.3 * width, 0.4 * height, 0.7 * width, 0.47 * height, 20, 20);
	fillroundrect(0.3 * width, 0.52 * height, 0.7 * width, 0.59 * height, 20, 20);
	//设置矩形框内的字体:
	settextcolor(WHITE);
	settextstyle(width * 0.047, 0, "SIMYOU");//幼圆
	outtextxy(0.37 * width, 0.41 * height, "单 人 游 戏");
	outtextxy(0.37 * width, 0.53 * height, "双 人 游 戏");
	//设置版本字体样式：
	settextcolor(RED);
	settextstyle(width * 0.03, 0, "calibri");
	outtextxy(0.6 * width, 0.35 * height, "Version:1.1.2");
	//画右下角的矩形：
	setfillcolor(BRICK);
	fillroundrect(0.9 * width, 0.9 * height, 0.95 * width, 0.95 * height, 20, 20);
	//右下角的字
	settextcolor(WHITE);
	settextstyle(0.03 * height, 0.03 * width, "Arvo-Regular", 0, 0, 50, 0, 0, 0);
	outtextxy(0.91 * width, 0.91 * height, "M");
	//播放背景音乐：
	playbkmusic();
	//鼠标消息：
	MOUSEMSG M;

	while (1) {
		M = GetMouseMsg();
		//鼠标移至相应矩形时画出外矩形图
		if (M.x >= 0.3 * width && M.x <= 0.7 * width && M.y >= 0.4 * height && M.y <= 0.47 * height) {
			setlinecolor(RED);
			rectangle(0.29 * width, 0.39 * height, 0.71 * width, 0.48 * height);
			M = GetMouseMsg();
			if (M.uMsg == WM_LBUTTONDOWN) {
				gamemode = 1;//单人游戏
				break;
			}
		}
		else {
			setlinecolor(bkc);
			rectangle(0.29 * width, 0.39 * height, 0.71 * width, 0.48 * height);
		}//画一个与背景颜色相同的等大矩形来掩盖
		if (M.x >= 0.3 * width && M.x <= 0.7 * width && M.y >= 0.52 * height && M.y <= 0.59 * height) {
			setlinecolor(RED);
			rectangle(0.29 * width, 0.51 * height, 0.71 * width, 0.6 * height);
			M = GetMouseMsg();
			if (M.uMsg == WM_LBUTTONDOWN) {
				gamemode = 2;//双人游戏
				break;
			}
		}
		else {
			setlinecolor(bkc);
			rectangle(0.29 * width, 0.51 * height, 0.71 * width, 0.6 * height);
		}//画一个与背景颜色相同的等大矩形来掩盖

		//鼠标移动至M处
		if (M.x >= 0.9 * width && M.x <= 0.95 * width && M.y >= 0.9 * height && M.y <= 0.95 * height)
		{
			settextstyle(0.03 * height, 0.03 * width, "Arvo-Regular", 0, 0, 50, 0, 1, 0);//下划线
			outtextxy(0.91 * width, 0.91 * height, "M");
			while (1)
			{
				M = GetMouseMsg();
				if (M.uMsg == WM_LBUTTONDOWN) {
					clickTime++;
				}if (clickTime % 2 != 0) {
					closebkmusic();//鼠标点击M静音
				}
				else {
					playbkmusic();//再次点击时回复背景音乐
				}
				if (!(M.x >= 0.9 * width && M.x <= 0.95 * width && M.y >= 0.9 * height && M.y <= 0.95 * height))break;//鼠标移至M外处跳出循环
			}

		}
		else//鼠标移至M外处重画无下划线的M
		{
			setlinecolor(bkc);
			setfillcolor(BRICK);
			fillroundrect(0.9 * width, 0.9 * height, 0.95 * width, 0.95 * height, 20, 20);
			settextstyle(0.03 * height, 0.03 * width, "Arvo-Regular", 0, 0, 50, 0, 0, 0);//无下划线
			outtextxy(0.91 * width, 0.91 * height, "M");
		}
	}
	//做画面连接的时候不能有getchar（）！！！！！！！！！！！！
	//getchar();
	//及时关闭
	closegraph();
}
void drawStageMenu() {
	initgraph(unit * 34, unit * 26);
	//setbkcolor();
	loadimage(&stageMenu, ".\\image\\greybg.png", unit * 34, unit * 26);
	putimage(0, 0, &stageMenu);
	//游戏的提示说明：
	setbkmode(TRANSPARENT);
	settextcolor(ORANGE);
	settextstyle(30, 0, "方正粗黑宋简体");
	outtextxy(50, 30, "游戏按键说明：");
	outtextxy(50, 60, "按下J或K键选择关卡,按下H确认");
	outtextxy(50, 90, "使用WASD或方向键来操纵坦克，按下J或Enter来开炮");

	//关卡的文字输出
	settextcolor(BLACK);
	settextstyle(40, 0, "方正粗黑宋简体");
	outtextxy(unit * 34 * 0.3, unit * 26 * 0.5, "STAGE");
	sprintf_s(level, sizeof(level), "%d", stage);
	outtextxy(unit * 34 * 0.58, unit * 26 * 0.5, level);
	//关卡选择
	while (true)
	{
		char keydown = _getch();
		if (keydown == 'j' || keydown == 'J') {			//上一关
			if (stage == 1) {
				settextcolor(GREY);
				sprintf_s(level, sizeof(level), "%d", stage);
				outtextxy(unit * 34 * 0.58, unit * 26 * 0.5, level);
				stage = totalStage;
				settextcolor(BLACK);
				sprintf_s(level, sizeof(level), "%d", stage);
				outtextxy(unit * 34 * 0.58, unit * 26 * 0.5, level);
			}
			else {
				settextcolor(GREY);
				sprintf_s(level, sizeof(level), "%d", stage);
				outtextxy(unit * 34 * 0.58, unit * 26 * 0.5, level);
				stage--;
				settextcolor(BLACK);
				sprintf_s(level, sizeof(level), "%d", stage);
				outtextxy(unit * 34 * 0.58, unit * 26 * 0.5, level);

			}
		}
		else if (keydown == 'k' || keydown == 'K') {	//下一关
			if (stage == totalStage) {
				settextcolor(GREY);
				sprintf_s(level, sizeof(level), "%d", stage);
				outtextxy(unit * 34 * 0.58, unit * 26 * 0.5, level);
				stage = 1;
				settextcolor(BLACK);
				sprintf_s(level, sizeof(level), "%d", stage);
				outtextxy(unit * 34 * 0.58, unit * 26 * 0.5, level);
			}
			else
			{
				settextcolor(GREY);
				sprintf_s(level, sizeof(level), "%d", stage);
				outtextxy(unit * 34 * 0.58, unit * 26 * 0.5, level);
				stage++;
				settextcolor(BLACK);
				sprintf_s(level, sizeof(level), "%d", stage);
				outtextxy(unit * 34 * 0.58, unit * 26 * 0.5, level);
			}
		}
		else if (keydown == 'h' || keydown == 'H') {			//确定关卡开始游戏
			return;
		}
		Sleep(200);
	}
}
void drawGameMenu() {
	//game menu background
	putimage(unit * 26, 0, &gameMenuBackground);
	//show the current enemy number with pictures
	for (int i = 0; i < enemyNumber; i++) {
		int j = i % 2;
		putimage(unit * (3 + j) + unit * 26, unit * (2 + i/2), &enemyMark_hide, SRCAND);
		putimage(unit * (3 + j) + unit * 26, unit * (2 + i/2), &enemyMark, SRCPAINT);
		
	}

	//show the current HP of the two players
	putimage(3 * unit + unit * 26, 16 * unit, &playerMark1_hide, SRCAND);
	putimage(3 * unit + unit * 26, 16 * unit, &playerMark1, SRCPAINT);//player1
	putimage(3 * unit + unit * 26, 19 * unit, &playerMark2_hide, SRCAND);
	putimage(3 * unit + unit * 26, 19 * unit, &playerMark2, SRCPAINT);//player2
	putimage(3 * unit + unit * 26, 22 * unit, &flag_hide, SRCAND);
	putimage(3 * unit + unit * 26, 22 * unit, &flag, SRCPAINT);		  //flag

	//TEXT
	settextcolor(BLACK);
	settextstyle(unit * 2 / 3, 0, "方正粗黑宋简体", 0, 0, 300, 0, 0, 0);
	outtextxy(unit * 27, unit / 2, "Player I");
	outtextxy(unit * 27, unit, "Player II");
	//output score
	char* score1 = new char[20];
	char* score2 = new char[20];
	sprintf_s(score1, sizeof(score1), "%d", u1.getScore());
	sprintf_s(score2, sizeof(score2), "%d", u2.getScore());
	outtextxy(unit * 30, unit / 2, score1);
	outtextxy(unit * 30, unit, score2);

	//player's HP
	char* HP1 = new char[4];
	char* HP2 = new char[4];
	sprintf_s(HP1, sizeof(HP1), "%d", u1.getHP());
	sprintf_s(HP2, sizeof(HP2), "%d", u2.getHP());
	settextstyle(unit, 0, "方正粗黑宋简体");
	outtextxy(unit * 27, unit * 15, "P I");
	outtextxy(unit * 30, unit * 15, HP1);
	outtextxy(unit * 27, unit * 18, "P II");
	outtextxy(unit * 30, unit * 18, HP2);

	//stage
	outtextxy(unit * 32, unit * 23, level);
}
void crossStage() {
	loadimage(&stageMenu, "greybg.png", unit * 34, unit * 26);
	putimage(0, 0, &stageMenu);
	//关卡的文字输出
	settextcolor(BLACK);
	settextstyle(40, 0, "方正粗黑宋简体");
	outtextxy(unit * 34 * 0.3, unit * 26 * 0.5, "STAGE");
	sprintf_s(level, sizeof(level), "%d", stage);
	outtextxy(unit * 34 * 0.58, unit * 26 * 0.5, level);
}
void Gameover() {
	initgraph(unit * 34, unit * 26);
	loadimage(&gameEnd, ".\\image\\gameover.jpg", unit * 34, unit * 26);
	mciSendString("open .\\Audio\\GameOver.mp3 alias govermusic", 0, 0, 0);
	mciSendString("play govermusic", 0, 0, 0);
	int indeY = unit * 26;
	while (true)
	{
		putimage(0, indeY, &gameEnd);
		indeY -= unit / 2;
		if (indeY == 0) {
			Sleep(2000);
			break;
		}
		Sleep(30);	
	}
	
}

//************Input************
int getPlayer1Input() {
	if (KEY_DOWN('W') || KEY_DOWN('w')) {
		return UP;
	}
	if (KEY_DOWN('S') || KEY_DOWN('s')) {
		return DOWN;
	}
	if (KEY_DOWN('A') || KEY_DOWN('a')) {
		return LEFT;
	}
	if (KEY_DOWN('D') || KEY_DOWN('d')) {
		return RIGHT;
	}
	if (KEY_DOWN('J') || KEY_DOWN('j')) {
		return Fire;
	}
	return -1;
}
int getPlayer2Input() {
	if (KEY_DOWN(VK_UP))
		return UP;
	if (KEY_DOWN(VK_DOWN))
		return DOWN;
	if (KEY_DOWN(VK_LEFT))
		return LEFT;
	if (KEY_DOWN(VK_RIGHT))
		return RIGHT;
	if (KEY_DOWN(VK_RETURN))//回车键开火
		return Fire;
	return -1;
}

//*************Map******************
void putMap() {
	//show the dynamic water
	static int random = 0;
	static long counter = 1;
	counter++;
	if (counter % 30 == 0) {//change every 30*speed(ms);
		random = 1 - random;
		counter = 1;
	}
	putimage(0, 0, &background);
	for (int i = 0; i < map_x; i++) {
		for (int j = 0; j < map_y; j++) {
			switch (map[i][j])					//1-wall, 2-iron, 3/4-home, 6/7-sea, 8-border,9-ice,10-destroyed home
			{
			case 1:putimage(j * unit, i * unit, &wall);
				break;
			case 2:putimage(j * unit, i * unit, &iron);
				break;
			case 3:putimage(j * unit, i * unit, home_hide, SRCAND);
				putimage(j * unit, i * unit, home, SRCPAINT);
				break;
			case 6:putimage(j * unit, i * unit, sea + 1 - random);
				break;
			case 7:putimage(j * unit, i * unit, sea + random);
				break;
			case 8:putimage(j * unit, i * unit, &border);
				break;
			case 9:putimage(j * unit, i * unit, &ice);
				break;
			case 10:putimage(j * unit, i * unit, home_hide + 1, SRCAND);
				putimage(j * unit, i * unit, home + 1, SRCPAINT);
				break;
			default:
				break;
			}
		}
	}
}
void putAll1() {//资源1
	putMap();
	/********born effect*******/
	/*****user1****/
	static int temp1 = u1.getHP();
	static int temp2 = u2.getHP();
	if (stagechange == 1 || (temp1 != u1.getHP()) && (u1.getHP() > 0)) {
		u1.setCounter(1);
		u2.setCounter(1);
		u1.setBornDone(0);
		u2.setBornDone(0);
	}
	if (u1.getCounter() <= 16 || (temp1 != u1.getHP()) && (u1.getHP() > 0) || stagechange == 1) {
		temp1 = u1.getHP();
		u1.bornEffect();
		if (u1.getCounter() == 16) {
			u1.setBornDone(1);
		}
	}
	u1.setCounter(u1.getCounter() + 1);
	if (u1.getBornDone())
		u1.show();
	/*****user2****/
	if ((u2.getCounter() <= 16 || (temp1 != u2.getHP()) && (u2.getHP() > 0) || stagechange == 1) && gamemode == 2) {
		temp2 = u2.getHP();
		u2.bornEffect();
		if (u2.getCounter() == 16) {
			u2.setBornDone(1);
		}
	}
	u2.setCounter(u1.getCounter() + 1);
	if (u2.getBornDone()&& gamemode == 2)
		u2.show();
	/******bullet******/
	if (u1.getShootState() && u1.getBulletState()) {
		u1.showBullet();
	}
	if (u2.getShootState() && u2.getBulletState()) {
		u2.showBullet();
	}
	/*enemy Tank*/
	tank.show();
	tank2.show();
}
void putAll2() {//资源2
	tank.showBullet();
	tank2.showBullet();
	u1.showBoom();
	u2.showBoom();
	eTank->showBoom();
	tank2.showBoom();
	tank.showBoom();
	putjungle();
	putimage(unit * 26, 0, &gameMenuBackground);
	drawGameMenu();
}
void renewPic() {//刷新图片
	enemyTank* eTank = new enemyTank[enemyNumber];
	clock_t begintime = clock();
	while (true)
	{
		/***********************************************************/
		if (stagechange == 1) {
			//初始化敌方的坦克等级（仅在游戏的关卡变动时进行）
			for (int i = 0; i < enemyNumber; i++) {
				if (stage * i < enemyNumber / 3 * 2)
					(eTank + i)->setType(enemyType::normal);
				else if (stage * i < enemyNumber / 4 * 3)
					(eTank + i)->setType(enemyType::stronger);
				else if (stage * i < enemyNumber / 8 * 7)
					(eTank + i)->setType(enemyType::moveFaster);
				else(eTank + i)->setType(enemyType::shootFaster);
				(eTank + i)->setSeed(i * 10);
				(eTank + i)->loadTank();
				(eTank + i)->loadBullet();
				(eTank + i)->born();
			}
			tank2.loadBullet(); tank2.loadTank(); tank2.born();
			tank.loadBullet(); tank.loadTank(); tank.born();
		}
		/***********************************************************/

		int Oper1 = getPlayer1Input();				   //player1's operation
		int Oper2 = getPlayer2Input();
		u1.move(Oper1);
		u2.move(Oper2);
		int shoottime = 0;
	
		/*********************************************************************************/
		eTank->move(eTank->findDir(60));
		tank.move(tank.findDir(500));
		tank2.move(tank2.findDir(1000));
		static int autoshoot = 0;
		autoshoot++;
		/*********************************************************************************/
		BeginBatchDraw();	
		if ((clock() - begintime) % refleshFreq == 0) {//refresh with a certain frequency
			if (Oper1 == Fire && clock() - shoottime >= 150) {//玩家1开火
				shoottime = clock();
				u1.shoot();
				putAll1();
				eTank->show();
				enemyNumber--;
				if (autoshoot % 100 == 0) {
					eTank->shoot();
				}
				if ((autoshoot - rand() % 200 - 80) == 0) {
					tank2.shoot();
				}
				if ((autoshoot - rand() % 160 + 20) == 0) {
					tank.shoot();
				}
				eTank->showBullet();
				putAll2();
			}
			else
			{
				putAll1();
				eTank->show();
				if (autoshoot % 100 == 0) {
					eTank->shoot();
				}
				if ((autoshoot % (rand() % 120 + 40)) == 0) {
					tank2.shoot();
				}
				if ((autoshoot - rand() % 160 + 20) == 0) {
					tank.shoot();
				}
				eTank->showBullet();
				putAll2();
			}
		}
		stagechange = 0;
		Sleep(1);	
		EndBatchDraw();
		Oper1 = -1;
		Oper2 = -1;
		if (enemyNumber == 0) {//击败所有敌人，进入下一关
			Sleep(1000);
			stagechange = 1;
			stage++;
			if (stage > totalStage) {
				stage = 1;
			}
			//为下关将数据重新初始化
			u1.setTankX(u1.getBirthX());
			u1.setTankY(u1.getBirthY());
			u1.setCurDir(UP);
			u1.setShootState(0);
			u1.setBulletState(0);
			u1.clearBoom();
			u2.setTankX(u2.getBirthX());
			u2.setTankY(u2.getBirthY());
			u2.setShootState(0);
			u2.setBulletState(0);
			u2.setCurDir(UP);
			u2.clearBoom();
			tank.clearBoom();
			tank2.clearBoom();
			(eTank)->clearBoom();
			(eTank + 1)->clearBoom();
			enemyNumber = 14;
			loadMap(); //load next stage map array from file
			cleardevice();
			BeginBatchDraw();
			crossStage();
			EndBatchDraw();
			Sleep(800);
		}
		if (gameover == 1||(u1.getHP()==0)&&(u2.getHP()==0)) {//游戏是否结束
			putAll1();
			putAll2();
			Sleep(800);
			break;
		}
	}
}
void beginGame() {
	//菜单
	drawMenu(unit * 34, unit * 26, BLACK);
	u1.setHP(3);
	if (gamemode == 1) {
		u2.setHP(0);			//单人模式玩家二的生命数为0
	}
	else {
		u2.setHP(3);
	}
	//stage choose & how to play menu
	drawStageMenu();
	//游戏界面
	initgraph(unit * 34, unit * 26);
	closebkmusic();
	setbkmode(TRANSPARENT);
	//加载游戏的资源
	loadGameResource();
	//切换输入法：
	//ChangeLanguage();
	//刷新界面
	renewPic();
	Gameover();
}

int main() {
	beginGame();
}