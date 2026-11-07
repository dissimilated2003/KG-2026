#pragma once


#ifdef max
#undef max
#endif 


#include "GeometryObjectWithInitialTransformImpl.h"


#include "Ray.h"
#include "Intersection.h"
#include "stdafx.h"


#include <algorithm>
#include <complex>
#include <vector>
#include <cassert>


static constexpr double EPSILON = 1e-6;

bool IsZero(double x, double eps = EPSILON);
void AddUniqueRoot(std::vector<double>& roots, double root);
void AddRealRoot(std::vector<double>& roots, std::complex<double> const& root);
std::vector<double> SolveQuadratic(double a, double b, double c);
std::vector<double> SolveQuarticByFerrariMethod(double a4, double a3, double a2, double a1, double a0);


//класс №Тор"
class Torus : public CGeometryObjectWithInitialTransformImpl
{
public:
	Torus(
		double majorRadius,
		double tubeRadius,
		CVector3d const& center = CVector3d(),
		CMatrix4d const& transform = CMatrix4d()
	)
		: CGeometryObjectWithInitialTransformImpl(transform)
		, m_majorRadius(majorRadius)
		, m_tubeRadius(tubeRadius)
	{
		assert(majorRadius > 0);
		assert(tubeRadius > 0);

		CMatrix4d initialTransform;
		initialTransform.Translate(center.x, center.y, center.z);
		SetInitialTransform(initialTransform);
	}

	virtual bool Hit(CRay const& ray, CIntersection& intersection) const
	{
		// Вместо преобразования тора выполняем обратное преобразование луча
		// Результат будет тот же самый, но вычислить его будет проще
		CRay invRay = Transform(ray, GetInverseTransform());

		CVector3d const& start = invRay.GetStart();
		CVector3d const& dir = invRay.GetDirection();

		double R = m_majorRadius;
		double r = m_tubeRadius;

		//тор имеет уравнение: 
		//  (x^2 + y^2 + z^2 + R^2 − r^2)^2  −  4R^2 * (x^2 + y^2) = 0

		//обозначим R^2 - r^2 за k, тогда :
		//	(x^2 + y^2 + z^2 + k)^2  −  4R^2 * (x^2 + y^2) = 0
		double k = R * R - r * r;
		
		//начало луча:		S = (Sx, Sy, Sz)
		//направление луча:	c = (cx, cy, cz)
		//уравнение луча:	P(t) = S + c*t
		
		//Для вычисления точки пересечения луча с данным тором необходимо подставить
		//точку, лежащую на луче (P=S+c*t) в уравнение тора

		//x^2 + y^2 + z^2 - это то же самео, что и Dot(P,P);
		//x^2 + y^2 - квадрат расстояния от оси Z

		//введем 2 полинома A(t), B(t):
		//A(t) = x^2 + y^2 + z^2 + k
		//B(t) = x^2 + y^2

		//перепишем уравнение:
		//	(A(t))^2 - 4R^2 * B(t) = 0

		//подставим компоненты уравнения луча (из P = S + c*t):
		//	Dot(P,P) = |S|^2 + 2(S*c)*t + |c|^2*t^2
		//	x^2 + y^2 = (Sx^2 + Sy^2) + 2(Sx*cx + Sy*cy)*t + (cx^2 + cy^2)*t^2
		
		//и оба они имеют вид:   
		//	A2*t^2 + A1*t + A0
		//поэтому оба квадратичны относительно t

		double A2 = Dot(dir, dir);			//коэф при t^2
		double A1 = 2.0 * Dot(start, dir);	//коэф при t
		double A0 = Dot(start, start) + k;	//свободный член, k - геометрия тора

		double B2 = dir.x * dir.x + dir.y * dir.y;	//для B(t) также: коэф при t^2
		double B1 = 2.0 * (start.x * dir.x + start.y * dir.y);		//коэф при t
		double B0 = start.x * start.x + start.y * start.y;			//свободный член

		//A(t)^2 = (A2*t^2 + A1*t + A0)^2 =
		// = A2^2*t^4 + 2*A2*A1*t^3 + (A1^2 + 2*A2*A0)*t^2 + 2*A1*A0*t + A0^2

		//4 * R^2 * B(t) = 4*R^2 * (B2*t^2 + B1*t + B0) 

		double C4 = A2 * A2;
		double C3 = 2.0 * A2 * A1;
		double C2 = A1 * A1 + 2.0 * A2 * A0 -	4.0 * R * R * B2;
		double C1 = 2.0 * A1 * A0 -				4.0 * R * R * B1;
		double C0 = A0 * A0 -					4.0 * R * R * B0;
	
		//метод Феррари решает уравнение вида: 
		//	C4*t^4 + C3*t^3 + C2*t^2 + C1*t + C0 = 0
		std::vector<double> roots = SolveQuarticByFerrariMethod(C4, C3, C2, C1, C0);

		for (double hitTime : roots)
		{
			if (hitTime <= EPSILON)
			{
				continue;
			}

			// Вычисляем координаты точки столкновекния в мировой системе координат 
			// и системе координат объекта
			CVector3d hitPoint0 = ray.GetPointAtTime(hitTime);
			CVector3d hitPointInObjectSpace = invRay.GetPointAtTime(hitTime);

			double x = hitPointInObjectSpace.x;
			double y = hitPointInObjectSpace.y;
			double z = hitPointInObjectSpace.z;
			double At = x * x + y * y + z * z + R * R - r * r;

			//рассчет нормалей - вектор частных производных
			CVector3d hitNormalInObjectSpace(
				//dF/dx = 2 * (x^2 + y^2 + z^2 + R^2 - r^2) * 2x - 8 * R^2 * x =
				// = 2 * (At) * 2x - 8 * R^2 * x
				4.0 * x * (At - 2.0 * R * R),

				//dF/dy = 2 * (x^2 + y^2 + z^2 + R^2 - r^2) * 2y - 8 * R^2 * y =
				// = 2 * (At) * 2y - 8 * R^2 * y
				4.0 * y * (At - 2.0 * R * R),

				//dF/dz = 2 * (x^2 + y^2 + z^2 + R^2 - r^2) * 2z - 0 = 2 * (At) * 2z 
				4.0 * z * At
			);
			// Вычисляем нормаль в точке соударения
			CVector3d hitNormal0 = GetNormalMatrix() * hitNormalInObjectSpace;
			// Создаем и добавляем новую точку пересечения
			CHitInfo hit0{
				hitTime,
				*this,
				hitPoint0,
				hitPointInObjectSpace,
				hitNormal0,
				hitNormalInObjectSpace
			};

			intersection.AddHit(hit0);
		}

		return intersection.GetHitsCount() > 0;
	}

private:
	double m_majorRadius{};
	double m_tubeRadius{};
};


//аналитический метод нахождения точек пересечения луча с тором, метод Феррари
std::vector<double> SolveQuarticByFerrariMethod(
	double a4,
	double a3,
	double a2,
	double a1,
	double a0
)
{
	std::vector<double> roots{};
	if (IsZero(a4))
	{
		return roots;
	}

	double A = a3 / a4; //было: C4*t^4 + C3*t^3 + C2*t^2 + C1*t + C0 = 0
	double B = a2 / a4;
	double C = a1 / a4;	//стало: t^4 + A*t^3 + B*t^2 + C*t + D = 0 
	double D = a0 / a4;

	double A2 = A * A;
	double A3 = A * A * A;
	double A4 = A * A * A * A;

	//подстановка t = y - A/4 для того, чтобы убрать кубический член
	//и получить неполное квартальное уравнение вида 
	//y^4 + p*y^2 + q*y + r = 0
	//при помощи сдвига Чирнгауза:
	//			A_n-1		A
	// t = y - ------- = -------
	//			  n         4
	double p = B - 3.0 * A2 / 8.0;
	double q = C - A * B / 2.0 + A3 / 8.0;
	double r = D - A * C / 4.0 + A2 * B / 16.0 - 3.0 * A4 / 256.0;

	double shift = -A / 4.0;

	//если уравнение стало биквадратным (q=0): y^4 + p*y^2 + r = 0
	//то оно квадратное относительно y^2 = z: z^2 + pz + r = 0
	if (IsZero(q))
	{
		//решаем квадратное уравнение, получаем до двух корней z
		std::vector<double> y2Roots = SolveQuadratic(1.0, p, r);
		for (double y2 : y2Roots)
		{
			if (y2 > EPSILON)
			{
				// z > 0 - два действительных корня shift + sqrt(z), shift - sqrt(z)
				double y = sqrt(y2);
				AddUniqueRoot(roots, shift - y);
				AddUniqueRoot(roots, shift + y);
			}
			else if (IsZero(y2))
			{
				// z == 0 - один корень shift
				AddUniqueRoot(roots, shift);
			}
		}

		std::sort(roots.begin(), roots.end());
		return roots;
	}

	//оборачиваем в комплексные числа p, q, r, т.к при вычислении резольвенты 
	//может быть ситуация, когда мы посчитаем sqrt от отрицательного выражения
	std::complex<double> cp{ p, 0 };
	std::complex<double> cq{ q, 0 };
	std::complex<double> cr{ r, 0 };

	std::complex<double> ferP = -cp * cp / 12.0 - cr;
	std::complex<double> ferQ = -cp * cp * cp / 108.0 + cp * cr / 3.0 - cq * cq / 8.0;
	std::complex<double> ferR = -ferQ / 2.0 + sqrt(ferQ * ferQ / 4.0 + ferP * ferP * ferP / 27.0);
	std::complex<double> ferU = pow(ferR, 1.0 / 3.0);

	std::complex<double> y;
	if (std::abs(ferU) < EPSILON)
	{
		y = -5.0 * cp / 6.0 - pow(ferQ, 1.0 / 3.0);
	}
	else
	{
		y = -5.0 * cp / 6.0 + ferU - ferP / (3.0 * ferU);
	}

	std::complex<double> W = sqrt(cp + 2.0 * y);
	if (std::abs(W) < EPSILON)
	{
		return roots;
	}

	std::complex<double> firstPart = -(3.0 * cp + 2.0 * y + 2.0 * cq / W);
	std::complex<double> secondPart = -(3.0 * cp + 2.0 * y - 2.0 * cq / W);
	std::complex<double> root1 = shift + (W + sqrt(firstPart)) / 2.0;
	std::complex<double> root2 = shift + (W - sqrt(firstPart)) / 2.0;
	std::complex<double> root3 = shift + (-W + sqrt(secondPart)) / 2.0;
	std::complex<double> root4 = shift + (-W - sqrt(secondPart)) / 2.0;

	AddRealRoot(roots, root1);
	AddRealRoot(roots, root2);
	AddRealRoot(roots, root3);
	AddRealRoot(roots, root4);
	std::sort(roots.begin(), roots.end());

	return roots;
}


bool IsZero(double x, double eps)
{
	return std::abs(x) < eps;
}


void AddUniqueRoot(std::vector<double>& roots, double root)
{
	for (double existingRoot : roots)
	{
		if (std::abs(existingRoot - root) < EPSILON)
		{
			return;
		}
	}

	roots.push_back(root);
}


void AddRealRoot(std::vector<double>& roots, std::complex<double> const& root)
{
	if (std::abs(root.imag()) < EPSILON)
	{
		AddUniqueRoot(roots, root.real());
	}
}


std::vector<double> SolveQuadratic(double a, double b, double c)
{
	std::vector<double> roots;

	if (IsZero(a))
	{
		if (!IsZero(b))
		{
			roots.push_back(-c / b);
		}
		return roots;
	}

	double discriminant = b * b - 4 * a * c;
	if (discriminant < -EPSILON)
	{
		return roots;
	}

	if (IsZero(discriminant))
	{
		roots.push_back(-b / (2 * a));
		return roots;
	}

	double sqrtDiscriminant = sqrt(std::max(0.0, discriminant));
	roots.push_back((-b - sqrtDiscriminant) / (2 * a));
	roots.push_back((-b + sqrtDiscriminant) / (2 * a));
	return roots;
}
