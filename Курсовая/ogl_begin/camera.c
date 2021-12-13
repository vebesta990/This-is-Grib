//#include <Windows.h>
#include <GL/freeglut.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "camera.h"
#include "mmsystem.h"
#pragma comment(lib,"Winmm.lib")

int keyStart = 0;	   // ��� ������ ����, ������ ����

// ����������� ������������� ��������� ������
struct SCamera camera = { 0, 0, 1.7, 0, 0 };

// ������� ���������� ���������� ��������� ������
void Camera_Apply()
{
	glRotatef(-camera.Xrot, 1, 0, 0);					// ��������� �� x �� ���� -camera.Xrot
	glRotatef(-camera.Zrot, 0, 0, 1);					// ��������� �� z �� ���� -camera.Zrot
	glTranslatef(-camera.x, -camera.y, -camera.z);		// �������� ������ 
}

// ������� �������� ������
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

// ������� �������� ������ � ������� �������
void Camera_AutoMoveByMouse(int centerX, int centerY, float speed)
{
	POINT cur;																// ����� ���-�� �� ������ (������� ������� 2) 
	POINT base = { centerX, centerY };										// ����� � ������ (������� ������� 1)
	GetCursorPos(&cur);														// ���������� ������� �������
	Camera_Rotation((base.y - cur.y) * speed, (base.x - cur.x) * speed);	// ������ �� ������� ��������� ������ �� ������ � ������������ ������ �� ���� ���� 
	SetCursorPos(base.x, base.y);											// ������������� ������ � ����� ������
}

// ������� �������� �������������� ��������� ��������� ������������� �����
BOOL IsCoordInMap(float x, float y)
{
	return (x >= 0) && (x < W) && (y >= 0) && (y < H);
}

// ������� ����������� ������
void Camera_MoveDirection(int forwardMove, int rightMove, float speed)
{
	// ������������� �������� ������ �� ������ ����
	if (keyStart == 0) {	// ����� 1 ��� ������ � ���� ����
		camera.x = 500;		// ��������� ������� ������
		camera.y = 500;		// ��������� ������� ������
		keyStart++;
	}

	float ugol = -camera.Zrot / 180 * M_PI;											// ����������� ���� �� �������� ������� ����� � ������� 
																					// forwardMove ��� ������ � �����, � rightMove ������ � �����

	if (forwardMove > 0)															// ���� ���� ������
		ugol += rightMove > 0 ? M_PI_4 : (rightMove < 0 ? -M_PI_4 : 0);				// ���� ������ 0 ���� ������+������, ����� �����+������, ����� ������ ������
	if (forwardMove < 0)															// ���� ���� �����
		ugol += M_PI + (rightMove > 0 ? -M_PI_4 : (rightMove < 0 ? M_PI_4 : 0));	// ���� ������ 0 ���� ������+�����, ����� �����+�����, ����� ������ �����
	if (forwardMove == 0) {															// ���� ����� �� ���� �� ������, �� �����
		ugol += rightMove > 0 ? M_PI_2 : -M_PI_2;									// �� ����, ���� ������ 0, �� ������, ����� �����
		if (rightMove == 0)	{														// ���� ������ ����� �� �����
			speed = 0;																// �������, ��� �������� ����� 0, ����� �� �����
			PlaySound(NULL, NULL, 0);												// ���� �� ����������, �� ���������� ��������, ����� ����� ������
		}
	}
	if (speed != 0) {																// ���� �������� �� ����� ����
		if (IsCoordInMap(camera.x + sin(ugol) * speed, camera.y + cos(ugol) * speed)) {	// � ����, �� ����� ���� �� ���� �� ��� ��������� ����� SCP (�������)
			PlaySound("C:/Users/����/Desktop/������/Graphics/Projekt/��������/01.wav", NULL, SND_ASYNC || SND_LOOP);	// ���� ���� ������ (�����)
			camera.x += sin(ugol) * speed;											// ���, �� ���� ����-�� �� x � ��������� �� ��������
			camera.y += cos(ugol) * speed;											// ���, �� ���� ����-�� �� y � ��������� �� ��������
		}
		else																		// ��������� ������ �������, ���� ������
			PlaySound(NULL, NULL, 0);												// ��� ��� ��������� �� ����, � �� ��������, ��� �� �������� ��� ��������������
	}
}