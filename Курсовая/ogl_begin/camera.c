//#include <Windows.h>
#include <GL/freeglut.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "camera.h"
#include "mmsystem.h"
#pragma comment(lib,"Winmm.lib")

int keyStart = 0;	   // Для старта надо, просто ключ

// Стандартная инициализация положения камеры
struct SCamera camera = { 0, 0, 1.7, 0, 0 };

// Функция применения параметров положения камеры
void Camera_Apply()
{
	glRotatef(-camera.Xrot, 1, 0, 0);					// Повернуть по x на угол -camera.Xrot
	glRotatef(-camera.Zrot, 0, 0, 1);					// Повернуть по z на угол -camera.Zrot
	glTranslatef(-camera.x, -camera.y, -camera.z);		// Сместить камеру 
}

// Функция поворота камеры
void Camera_Rotation(float xAngle, float zAngle)
{
	camera.Zrot += zAngle;
	if (camera.Zrot < 0)
		camera.Zrot += 360;
	if (camera.Zrot > 360)
		camera.Zrot -= 360;
	camera.Xrot += xAngle;  
	if (camera.Xrot < 0)
		camera.Xrot = 0;
	if (camera.Xrot > 180)
		camera.Xrot = 180;
}

// Функция поворота камеры с помощью курсора
void Camera_AutoMoveByMouse(int centerX, int centerY, float speed)
{
	POINT cur;																// Точка где-то на экране (позиция курсора 2) 
	POINT base = { centerX, centerY };										// Точка в центре (позиция курсора 1)
	GetCursorPos(&cur);														// Глобальная позиция курсора
	Camera_Rotation((base.y - cur.y) * speed, (base.x - cur.x) * speed);	// Узнаем на сколько сместился курсор от центра и поварачиваем камеру на этот угол 
	SetCursorPos(base.x, base.y);											// Устанавливаем курсор в центр экрана
}

// Функция проверки принадлежности координат диапозону существования карты
BOOL IsCoordInMap(float x, float y)
{
	return (x >= 0) && (x < W) && (y >= 0) && (y < H);
}

// Функция перемещения камеры
void Camera_MoveDirection(int forwardMove, int rightMove, float speed)
{
	// Инициализация поожения игрока на старте игры
	if (keyStart == 0) {	// Всего 1 раз зайдем в этот цикл
		camera.x = 500;		// Начальная позиция игрока
		camera.y = 500;		// Начальная позиция игрока
		keyStart++;
	}

	float ugol = -camera.Zrot / 180 * M_PI;											// Преобразуем угол по которому смотрит игрок в радианы 
																					// forwardMove это вперед и назад, а rightMove вправо и влево

	if (forwardMove > 0)															// Если идем вперед
		ugol += rightMove > 0 ? M_PI_4 : (rightMove < 0 ? -M_PI_4 : 0);				// Если больше 0 идем вправо+вперед, иначе влево+вперед, иначе просто вперед
	if (forwardMove < 0)															// Если идем назад
		ugol += M_PI + (rightMove > 0 ? -M_PI_4 : (rightMove < 0 ? M_PI_4 : 0));	// Если больше 0 идем вправо+назад, иначе влево+назад, иначе просто назад
	if (forwardMove == 0) {															// Если стоим не идем ни вперед, ни назад
		ugol += rightMove > 0 ? M_PI_2 : -M_PI_2;									// То идем, если больше 0, то вправо, иначе влево
		if (rightMove == 0)	{														// Если вообще стоим на месте
			speed = 0;																// Логично, что скорость равна 0, когда мы стоим
			PlaySound(NULL, NULL, 0);												// Звук на беззвучный, не привлекаем внимания, грибы любят тишину
		}
	}
	if (speed != 0) {																// Если скорость не равна нулю
		if (IsCoordInMap(camera.x + sin(ugol) * speed, camera.y + cos(ugol) * speed)) {	// И если, то место куда мы идем не под контролем фонда SCP (граница)
			PlaySound("C:/Users/Рома/Desktop/ПетрГУ/Graphics/Projekt/Курсовая/01.wav", NULL, SND_ASYNC || SND_LOOP);	// Типо звук ходьбы (почти)
			camera.x += sin(ugol) * speed;											// Ура, мы идем куда-то по x с поправкой на скорость
			camera.y += cos(ugol) * speed;											// Вау, мы идем куда-то по y с поправкой на скорость
		}
		else																		// Аккуратно дальше граница, идти нельзя
			PlaySound(NULL, NULL, 0);												// Так что приглушим мы шаги, а то подумают, что мы нелегалы или контрабандисты
	}
}