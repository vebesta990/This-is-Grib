#define W 1000 // Разметка границы равно mapW
#define H 1000 // Разметка границы равно mapH 

// Структура (модуль) камеры
struct SCamera {
	float x, y, z;
	float Xrot, Zrot;
} camera;

// Функция применения параметров положения камеры
void Camera_Apply();
// Функция поворота камеры
void Camera_Rotation(float xAngle, float zAngle);
// Функция поворота камеры курсором
void Camera_AutoMoveByMouse(int centerX, int centerY, float speed);
// Функция перемещения камеры
void Camera_MoveDirection(int forwardMove, int rightMove, float speed);
// Грибников и прочих на территорию фонда SCP не пускать (Контроль прохода через границу)
BOOL IsCoordInMap(float x, float y);