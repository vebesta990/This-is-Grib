//#include <Windows.h>
#include <GL/freeglut.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include <GL/stb_image.h>
#include "main.h"
#include "camera.h"

#include "mmsystem.h"
#pragma comment(lib,"Winmm.lib")

// Функция загрузки текстуры
void LoadTexture(char* file_name, int *target)
{
	int width, height, cnt;
	unsigned char* data = stbi_load(file_name, &width, &height, &cnt, 0);				// Загрузка файла текстуры
	glGenTextures(1, target);															// Генерация текстуры 
	glBindTexture(GL_TEXTURE_2D, *target);												// Сделать текстуру активной

	// Настройки текстуризации:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, cnt == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);

	glBindTexture(GL_TEXTURE_2D, 0);                                                // Дезактивировать текстуру
	stbi_image_free(data);                                                          // Освободить из памяти
}


#define sqr(a) (a)*(a)
void CalcNormals(TCell a, TCell b, TCell c, TCell* n)
{
	float wrki;
	TCell v1, v2;

	v1.x = a.x - b.x;
	v1.y = a.y - b.y;
	v1.z = a.z - b.z;
	v2.x = b.x - c.x;
	v2.y = b.y - c.y;
	v2.z = b.z - c.z;

	n->x = (v1.y * v2.z - v1.z * v2.y);
	n->y = (v1.z * v2.x - v1.z * v2.z);
	n->z = (v1.x * v2.y - v1.y * v2.x);
	wrki = sqrt(sqr(n->x) + sqr(n->y) + sqr(n->z));
	n->x /= wrki;
	n->y /= wrki;
	n->z /= wrki;
}

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

// Настройки анимации
void Anim_Set(TAnim* anm, TObject* obj)
{
	if (anm->obj != NULL)
		return;
		anm->obj = obj;
		anm->cnt = 10;
		anm->dx = (camera.x - obj->x) / (float)anm->cnt;
		anm->dy = (camera.y - obj->y) / (float)anm->cnt;
		anm->dz = ((camera.z - obj->scale - 0.1) - obj->z) / (float)anm->cnt;
}

// Анимация сбора
void Anim_Move(TAnim* anm)
{
	if (anm->obj != NULL) {
		anm->obj->x += anm->dx;
		anm->obj->y += anm->dy;
		anm->obj->z += anm->dz;
		anm->cnt--;
		if (anm->cnt < 1) {
			int i;
			for (i = 0; i < bagSize; i++)
				if (bag[i].type < 0) {
					bag[i].type = anm->obj->type;
					break;
				}  
			if (i < bagSize) {
				anm->obj->x = rand() % mapW;
				anm->obj->y = rand() % mapH;
			}
			anm->obj->z = Map_GetHeight(anm->obj->x, anm->obj->y);
			anm->obj = NULL;
		}
	}
}

// Функция создания холмов, чтоб карта плоской не была
void Map_CreateHill(int posX, int posY, int rad, int height)
{
	for (int i = posX - rad; i <= posX + rad; i++)
		for (int j = posY - rad; j <= posY + rad; j++)
			if (IsCoordInMap(i, j)) {
				float len = sqrt(pow(posX - i, 2) + pow(posY - j, 2));
				if (len < rad) {
					len = len / rad * M_PI_2;
					map[i][j].z += cos(len) * height;
				}
			}
}

float Map_GetHeight(float x, float y)
{
	if (!IsCoordInMap(x, y))		// Проверка выхода за границы карты
		return 0;					// Ой, Кажется переступили за границу
	int cX = (int)x;
	int cY = (int)y;
	float h1 = ((1 - (x - cX)) * map[cX][cY].z + (x - cX) * map[cX+1][cY].z);
	float h2 = ((1 - (x - cX)) * map[cX][cY+1].z + (x - cX) * map[cX+1][cY+1].z);
	return (1 - (y - cY)) * h1 + (y - cY) * h2;
}

// Инициализация всего и вся
void Map_Init()
{
	// Инициализируем рюкзак
	for (int i = 0; i < bagSize; i++)
		bag[i].type = -1;

	// Загружаем текстуры объектов и текстуру карты
	LoadTexture("textures/pole.png", &tex_pole);
	LoadTexture("textures/trava.png", &tex_trava);
	LoadTexture("textures/flower.png", &tex_flower); 
	LoadTexture("textures/flower2.png", &tex_flower2);
	LoadTexture("textures/grib.png", &tex_grib);
	LoadTexture("textures/grib2.png", &tex_grib2);
	LoadTexture("textures/grib3.png", &tex_grib3);
	LoadTexture("textures/tree.png", &tex_tree);
	LoadTexture("textures/tree2.png", &tex_tree2);

	// Включаем подсветку
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);

	// И про альфа канал не забудем
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.99);

	// Пробегаем по всем вершинам карты и инициализируем
	// массив вершин и текстурные координаты
	for (int i = 0; i < mapW; i++)
		for (int j = 0; j < mapH; j++) {
			map[i][j].x = i;
			map[i][j].y = j;
			map[i][j].z = (rand() % 10) * 0.02;		// Создаем микрорельеф

			mapUV[i][j].u = i;
			mapUV[i][j].v = j;
		}
	// Инициализируем клетки (каждая клетка = 2 треугольника = 6 вершин)
	for (int i = 0; i < mapW - 1; i++) {
		int pos = i * mapH;
		for (int j = 0; j < mapH - 1; j++) {
			mapInd[i][j][0] = pos;
			mapInd[i][j][1] = pos + 1;
			mapInd[i][j][2] = pos + 1 + mapH;

			mapInd[i][j][3] = pos + 1 + mapH;
			mapInd[i][j][4] = pos + mapH;
			mapInd[i][j][5] = pos;

			pos++;
		}
	}

	// Создаем холмы
	for (int i = 0; i < 1000; i++)
		Map_CreateHill(rand() % mapW, rand() % mapH, 143 % 1000, rand() % 10);
	
	for (int i = 0; i < mapW - 1; i++)
		for (int j = 0; j < mapH - 1; j++)
			CalcNormals(map[i][j], map[i+1][j], map[i][j+1], &mapNormal[i][j]);

	// Задаем кол-во разных объектов
	int travaN = 40000;
	int gribN = 3000;
	int grib2N = 3000;
	int grib3N = 3000;
	int treeN = 4000;

	plantCnt = travaN + gribN + grib2N + grib3N + treeN;			// Всего объектов
	plantMas = realloc(plantMas, sizeof(*plantMas) * plantCnt);
	for (int i = 0; i < plantCnt; i++) {							// Проходим по всем объектам
		if (i < travaN) {											// Спавним траву и цветы
			plantMas[i].type = rand() % 10 != 0 ? tex_trava :
				(rand() % 20 == 0 ? tex_flower : tex_flower2);
			plantMas[i].scale = 0.7 + (rand() % 10) * 0.1;
		}
		else if (i < (travaN + gribN)) {								// Спавним грибы, которые обычные
			plantMas[i].type = tex_grib;
			plantMas[i].scale = 0.2 + (rand() % 50) * 0.01;
		}
		else if (i < (travaN + gribN + grib2N)) {						// Спавним необычные грибы
			plantMas[i].type = tex_grib2;
			plantMas[i].scale = 0.2 + (rand() % 50) * 0.01;
		}
		else if (i < (travaN + gribN + grib2N + grib3N)) {				// Спавним аномальные грибы
			plantMas[i].type = tex_grib3;
			plantMas[i].scale = 0.2 + (rand() % 50) * 0.01;
		}
		else {															// Спавним деревья
			plantMas[i].type = rand() % 2 == 0 ? tex_tree : tex_tree2;
			plantMas[i].scale = 4 + (rand() % 50);
		}
		plantMas[i].x = rand() % mapW;
		plantMas[i].y = rand() % mapH;
		plantMas[i].z = Map_GetHeight(plantMas[i].x, plantMas[i].y);	// Это чтоб то, что заспавнили не летало, а на земле стояло 
	}
	PlaySound("C:/Users/Рома/Desktop/ПетрГУ/Graphics/Projekt/Курсовая/start.wav", NULL, SND_FILENAME);	// Вступительная речь грибника
}

// Функция отрисовки всего и вся
void Map_Show()
{
	float sz = 0.1;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-scrKoef * sz, scrKoef * sz, -sz, sz, sz * 2, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);

	static float alfa = 0;
	alfa += 0.1;
	if (alfa > 180)
		alfa -= 360;

	#define abs(a) ((a) > 0 ? (a) : -(a))
	float kcc = 1 - (abs(alfa) / 180);

	#define sakat 40
	float k = 90 - abs(alfa);
	k = (sakat - abs(k));
	k = k < 0 ? 0 : k / sakat;

	if (selectMode)
		glClearColor(0, 0, 0, 0);
	else
		glClearColor(0.6f * kcc, 0.8f * kcc, 1.0f * kcc, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (selectMode) {
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
	} else {
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
	}

	Anim_Move(&animation);

	glPushMatrix();
	if (!selectMode) {
		glPushMatrix();
		glRotatef(-camera.Xrot, 1, 0, 0);
		glRotatef(-camera.Zrot, 0, 0, 1);
		glRotatef(alfa, 0, 1, 0);
		glTranslatef(0, 0, 20);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glColor3f(1, 1 - k * 0.8, 1 - k);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, sun);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_DEPTH_TEST);
		glPopMatrix();
	}
	Camera_Apply();

	glPushMatrix();
	glRotatef(alfa, 0, 1, 0);
	GLfloat position[] = { 0, 0, 1, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	
	float mas[] = { 1 + k * 2, 1, 1, 0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, mas);

	float clr = kcc * 0.15 + 0.05;
	float mas0[] = { clr, clr, clr, 0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, mas0);
	glPopMatrix();

	if (!selectMode) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, map);
		glTexCoordPointer(2, GL_FLOAT, 0, mapUV);
		glColor3f(0.7f, 0.7f, 0.7f);
		glNormalPointer(GL_FLOAT, 0, mapNormal);
		glBindTexture(GL_TEXTURE_2D, tex_pole);
		glDrawElements(GL_TRIANGLES, mapIndCnt, GL_UNSIGNED_INT, mapInd);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, plant);
	glTexCoordPointer(2, GL_FLOAT, 0, plantUV);
	glColor3f(1.0, 1.0, 1.0);
	glNormal3f(0, 0, 1);
	selectMasCnt = 0;
	int selectColor = 1;
	for (int i = 0; i < plantCnt; i++) {
		if (selectMode) {
			if ((plantMas[i].type == tex_tree) || (plantMas[i].type == tex_tree2) || (plantMas[i].type == tex_trava) || (plantMas[i].type == tex_flower) || (plantMas[i].type == tex_flower2))
				continue;
			static int radius = 3;
			if ((plantMas[i].x > camera.x - radius)
				&& (plantMas[i].x < camera.x + radius)
				&& (plantMas[i].y > camera.y - radius)
				&& (plantMas[i].y < camera.y + radius)) {
				glColor3ub(selectColor, 0, 0);
				selectMas[selectMasCnt].colorIndex = selectColor;
				selectMas[selectMasCnt].plantMas_Index = i;
				selectMasCnt++;
				selectColor++;
				if (selectColor >= 255)
					break;
			}
			else
				continue;
		}
		glBindTexture(GL_TEXTURE_2D, plantMas[i].type);
		glPushMatrix();
		glTranslatef(plantMas[i].x, plantMas[i].y, plantMas[i].z);
		glScalef(plantMas[i].scale, plantMas[i].scale, plantMas[i].scale);
		glDrawElements(GL_TRIANGLES, plantIndCnt, GL_UNSIGNED_INT, plantInd);
		glPopMatrix();
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glPopMatrix();
}

// Счётчик голода
void Hunger()
{
	static hunger = 0;				// Переменная счетчик голода
	hunger++;						// Ой-ёй чот кушать захотелось
	if (hunger > 100) {				// Ну все не могу терпеть
		hunger = 0;					// Хотя ...
		health--;					// Но это же вредно для здоровья 
		if (health < 1)				// ОХ...кххх...умираю
			PostQuitMessage(0);		// The END
	}
}

// Функция управления в игре
void Player_Move()
{
	Camera_MoveDirection(GetKeyState('W') < 0 ? 1 : (GetKeyState('S') < 0 ? -1 : 0)		//||>	Премещение игрока
		, GetKeyState('D') < 0 ? 1 : (GetKeyState('A') < 0 ? -1 : 0)					//|| >	 Если W, то вперед, если S, то назад, а иначе ничего
			, 1.0);																		//||>	Если D, то вправо, если A то влево, а иначе ничего
	if (mouseBind)											// Смотрим режим отображения курсора, TRUE скрыть курсор, FALSE показать и отключить поворот
		Camera_AutoMoveByMouse(400, 400, 0.2);				// Поворот камеры вслед за курсором
	camera.z = Map_GetHeight(camera.x, camera.y) + 3.0;		// Чтоб игрок по карте ходил, но расстояние камеры 3.0 от земли
}

// Функция сбора
void Player_Take(HWND hwnd)
{
	selectMode = TRUE;
	Map_Show();
	selectMode = FALSE;

	RECT rct;
	GLubyte clr[3];
	GetClientRect(hwnd, &rct);
	glReadPixels(rct.right / 2.0, rct.bottom / 2.0, 1, 1, GL_RGB,
		GL_UNSIGNED_BYTE, clr);
	if (clr[0] > 0) {
		for (int i = 0; i < selectMasCnt; i++)									// Проходим по массиву объектов сбора 
			if (selectMas[i].colorIndex == clr[0]) {							// А можно ли это взять?
				Anim_Set(&animation, plantMas + selectMas[i].plantMas_Index);	// Анимация сбора и сбор
				// Дима, звука не будет, собирать надо тихо (>_<)
			}
	}
}

// Функция изменения размеров окна (корректная перспектива и всё такое)
void WndResize(int x, int y)
{
	glViewport(0, 0, x, y);
	scrSize.x = x;
	scrSize.y = y;
	scrKoef = x / (float)y;
}

// Функция отрисовки рюкзака
void Bag_Show(int x, int y, int scale)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, bagRectUV);
	glTexCoordPointer(2, GL_FLOAT, 0, bagRectUV);
	for (int i = 0; i < bagSize; i++) {
		glPushMatrix();
		glTranslatef(x + i * scale, y, 0);
		glScalef(scale, scale, 1);
		glColor3ub(110, 95, 73);
		glDisable(GL_TEXTURE_2D);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		if (bag[i].type > 0) {
			glColor3f(1, 1, 1);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, bag[i].type);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		}

		glColor3ub(160, 146, 116);
		glLineWidth(3);
		glDisable(GL_TEXTURE_2D);
		glDrawArrays(GL_LINE_LOOP, 0, 4);
		glPopMatrix();
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

// Функция вкида грибами :)
void Bag_Click(int x, int y, int scale, int mx, int my)
{
	if ((my < y) || (my > y + scale))
		return;
	for (int i = 0; i < bagSize; i++) {
		if ((mx > x + i * scale) && (mx < x + (i + 1) * scale)) {
			if (bag[i].type == tex_grib) {
				health++;
				if (health > healthMax)
					health = healthMax;
			}
			if (bag[i].type == tex_grib2) {
				if (health > 5)
					health -= 3;
				else
					health = 2;
			}
			if (bag[i].type == tex_grib2) {
				if (health < 17)
					health += 3;
				else
					health = healthMax;
			}
			bag[i].type = -1;
		}
	}
}

// Функция отрисовки панели Health (HP)
void Health_Show(int x, int y, int scale)
{
	glDisable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, heart);
	for (int i = 0; i < healthMax; i++) {
		glPushMatrix();
		glTranslatef(x + i * scale, y, 0);
		glScalef(scale, scale, 1);
		if (i < health)
			glColor3f(1, 0, 0);
		else
			glColor3f(0, 0, 0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
		glPopMatrix();
	}
	glDisableClientState(GL_VERTEX_ARRAY);
}

// Функция отрисовки прицела
void Cross_Show()
{
	static float cross[] = { 0, -1, 0, 1, -1, 0, 1, 0 };
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, cross);
	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(scrSize.x * 0.5, scrSize.y * 0.5, 0);
	glScalef(15, 15, 1);
	glLineWidth(3);
	glDrawArrays(GL_LINES, 0, 4);
	glPopMatrix();
	glDisableClientState(GL_VERTEX_ARRAY);
}

// Вообще вся отрисовка меню
void Menu_Show()
{
	// Настраеваем проекции
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, scrSize.x, scrSize.y, 0, -1, 1);	// Перспектива (Оси координат OpenGL совпадают с осями клиентской области)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Отключаем подсветку
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	
	// Рисуем рюкзак
	Bag_Show(10, 10, 50);
	// Рисуем HP
	Health_Show(10, 70, 30);
	// Рисуем прицел
	Cross_Show();
}

// Настройка окна и прочее (стандартный шаблон)
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	WNDCLASSEX wcex;
	HWND hwnd;
	HDC hDC;
	HGLRC hRC;
	MSG msg;
	BOOL bQuit = FALSE;
	float theta = 0.0f;

	/* register window class */
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "GLSample";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


	if (!RegisterClassEx(&wcex))
		return 0;

	/* create main window */
	hwnd = CreateWindowEx(0,
		"GLSample",
		"Это всё грибы!",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1600,
		900,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, nCmdShow);
	SetCursor(wcex.hCursor);

	/* enable OpenGL for the window */
	EnableOpenGL(hwnd, &hDC, &hRC);

	RECT rct;
	GetClientRect(hwnd, &rct);
	WndResize(rct.right, rct.bottom);
	Map_Init();
	glEnable(GL_DEPTH_TEST);

	/* program main loop */
	while (!bQuit)
	{
		/* check for messages */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			/* handle or dispatch messages */
			if (msg.message == WM_QUIT)
			{
				bQuit = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			/* OpenGL animation code goes here */
			if (GetForegroundWindow() == hwnd) 
				Player_Move();
			Hunger();
			Map_Show();
			Menu_Show();
			SwapBuffers(hDC);
			Sleep(1);
		}
	}

	/* shutdown OpenGL */
	DisableOpenGL(hwnd, hDC, hRC);

	/* destroy the window explicitly */
	DestroyWindow(hwnd);

	return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_LBUTTONDOWN:
		if (mouseBind)
			Player_Take(hwnd);
		else
			Bag_Click(10, 10, 50, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_SIZE:
		WndResize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_SETCURSOR:
		ShowCursor(!mouseBind);
		break;

	case WM_DESTROY:
		return 0;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case 'E':
			mouseBind = !mouseBind;
			if (mouseBind)
				while (ShowCursor(FALSE) >= 0);
			else
				while (ShowCursor(TRUE) <= 0);
		}
	}
	break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
	PIXELFORMATDESCRIPTOR pfd;

	int iFormat;

	/* get the device context (DC) */
	*hDC = GetDC(hwnd);

	/* set the pixel format for the DC */
	ZeroMemory(&pfd, sizeof(pfd));

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	iFormat = ChoosePixelFormat(*hDC, &pfd);

	SetPixelFormat(*hDC, iFormat, &pfd);

	/* create and enable the render context (RC) */
	*hRC = wglCreateContext(*hDC);

	wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hwnd, hDC);
}

