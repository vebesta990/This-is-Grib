// ��������� ������� ������
typedef struct {
	float x, y, z;
} TCell;

// ��������� ��������� ��� ���������������
typedef struct {
	float u, v;
} TUV;

// ��������� ��������
typedef struct {
	float x, y, z;
	int type;
	float scale;
} TObject;

BOOL selectMode = FALSE;
#define ObjListCnt 255
// ��������� �������� �����
typedef struct {
	int plantMas_Index;
	int colorIndex;
} TSelectObj;
TSelectObj selectMas[ObjListCnt];
int selectMasCnt = 0;

// ��������� �������� �����
typedef struct {
	TObject* obj;
	float dx, dy, dz;
	int cnt;
} TAnim;
TAnim animation = { 0, 0, 0, 0, 0 };

// ��������� ������ � �������
typedef struct {
	int type;
} TSlot;
#define bagSize 12					// ���������� ������ ������ �������
TSlot bag[bagSize];					// ��� ������

#define mapW 1000					// ������ ������� �����
#define mapH 1000					// ������ ������� �����
TCell map[mapW][mapH];				// ���������� �����
TCell mapNormal[mapW][mapH];
TUV mapUV[mapW][mapH];

GLuint mapInd[mapW - 1][mapH - 1][6];					// ������ ��������
int mapIndCnt = sizeof(mapInd) / sizeof(GLuint);

float plant[] = { -0.5, 0, 0, 0.5, 0, 0, 0.5, 0, 1, -0.5, 0, 1,
				 0, -0.5, 0, 0, 0.5, 0, 0, 0.5, 1, 0, -0.5, 1 };
float plantUV[] = { 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0 };
GLuint plantInd[] = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4 };
int plantIndCnt = sizeof(plantInd) / sizeof(GLuint);

float bagRect[] = { 0, 0, 1, 0, 1, 1, 0, 1 };
float bagRectUV[] = { 0, 0, 1, 0, 1, 1, 0, 1 };

int health = 20;				// ��������
int healthMax = 20;				// ����������� ���������� ��������
float heart[] = { 0.5, 0.25, 0.25, 0, 0, 0.25, 0.5, 1, 1, 0.25, 0.75, 0 };	// ��������� ��������� ��������

int tex_pole, tex_trava, tex_flower, tex_flower2, tex_grib, tex_grib2, tex_grib3, tex_tree, tex_tree2;	// ������� ������������� �� �����

TObject* plantMas = NULL;
int plantCnt = 0;

float sun[] = { -1, -1, 0, 1, -1, 0, 1, 1, 0, -1, 1, 0 };

BOOL mouseBind = TRUE;			// ����� ����������� �������

POINT scrSize;
float scrKoef;