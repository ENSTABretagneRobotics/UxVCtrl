// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef COMPUTATIONS_H
#define COMPUTATIONS_H

#include "OSCore.h"
#include "rmatrix.h"
#include <deque>
#include <numeric>
#include <list>

class bound
{
public:
	double val;
	int opening;
	double mark;
	bound()
	{
		val = 0; opening = 0; mark = 0;
	}
	bound(const double& v, const int& o, const double& m)
	{
		val = v; opening = o; mark = m;
	}
	friend bool operator<(const bound &x, const bound &y)
	{
		return x.val < y.val;
	}
};
#pragma region C_ACC_GROUP
// Group number in ygroup should be from 0 to nbgroups-1.
inline void C_q_in_group(interval& x, int q, vector<interval>& y, vector<int>& ygroup, int nbgroups)
{
	vector<int> Vgroup(nbgroups); // >=0 when the group is allowed to add to the sum.
	vector<bound> V;
	if (x.isEmpty) return;
	if (q <= 0) return;
	for (unsigned int i = 0; i < y.size(); i++)
	{
		if (!y[i].isEmpty)
		{
			V.push_back(bound(y[i].inf, 1, ygroup[i]));
			V.push_back(bound(y[i].sup, -1, ygroup[i]));
		}
	}
	if (V.size() == 0)
	{
		x = interval();
		return;
	}
	V.push_back(bound(x.inf, 1, -1));
	V.push_back(bound(x.sup, -1, -1));
	sort(V.begin(), V.end());
	int sum = 0;
	int imin = (int)(V.size());
	int imax = -1;
	for (unsigned int i = 0; i < V.size(); i++)
	{
		if (V[i].opening < 0)
		{
			if (V[i].mark >= 0) Vgroup[(unsigned int)V[i].mark]++;
			if ((sum >= q)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
			{
				imax = i;			
			}
		}
		if (V[i].mark >= 0) if (Vgroup[(unsigned int)V[i].mark] >= 0) sum = sum + V[i].opening;
		if (V[i].opening > 0)
		{
			if (V[i].mark >= 0) Vgroup[(unsigned int)V[i].mark]--;
			if ((sum >= q)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
			{
				imin = min(imin, (int)i);
			}
		}
	}
	if ((imax == -1)||(imin == (int)(V.size()))) x = interval();
	else x = interval(V[imin].val, V[imax].val);
}

// Group number in ygroup should be from 0 to nbgroups-1.
inline void C_q_in_group(box& x, int q, vector<box>& yj, vector<int>& ygroup, int nbgroups)
{
	vector<box> y(yj.size());
	for (unsigned int j = 0; j < y.size(); j++)
	{
		y[j] = x&yj[j];
	}
	vector<interval> yi(y.size());
	for (int i = 1; i <= x.dim; i++)
	{
		for (unsigned int j = 0; j < y.size(); j++)
			yi[j] = y[j][i];
		C_q_in_group(x[i], q, yi, ygroup, nbgroups);
	}
}

inline void C_acc_group(interval& x, vector<interval>& y, vector<int>& ygroup, int nbgroups)
{
	for (unsigned int qi = y.size(); qi >= 1; qi--)
	{
		interval xi = x;
		C_q_in_group(xi, qi, y, ygroup, nbgroups);
		if (!xi.isEmpty) 
		{
			x = xi;
			return;
		}
	}
}

inline void C_acc_group(box& x, vector<box>& yj, vector<int>& ygroup, int nbgroups)
{
	vector<box> y(yj.size());
	for (unsigned int j = 0; j < y.size(); j++)
	{
		y[j] = x&yj[j];
	}
	vector<interval> yi(y.size());
	for (int i = 1; i <= x.dim; i++)
	{
		for (unsigned int j = 0; j < y.size(); j++)
			yi[j] = y[j][i];
		C_acc_group(x[i], yi, ygroup, nbgroups);
	}
}

// Group number in ygroup should be from 0 to nbgroups-1.
inline void C_acc_group_opt(interval& x, vector<interval>& y, vector<int>& ygroup, int nbgroups)
{
	vector<int> Vgroup(nbgroups); // >=0 when the group is allowed to add to the sum.
	vector<bound> V;
	if (x.isEmpty) return;
	for (unsigned int i = 0; i < y.size(); i++)
	{
		if (!y[i].isEmpty)
		{
			V.push_back(bound(y[i].inf, 1, ygroup[i]));
			V.push_back(bound(y[i].sup, -1, ygroup[i]));
		}
	}
	if (V.size() == 0)
	{
		x = interval();
		return;
	}
	V.push_back(bound(x.inf, 1, -1));
	V.push_back(bound(x.sup, -1, -1));
	sort(V.begin(), V.end());
	int q = -1;
	int sum = 0;
	int imin = (int)(V.size());
	int imax = -1;
	for (unsigned int i = 0; i < V.size(); i++)
	{
		if (V[i].opening < 0)
		{
			if (V[i].mark >= 0) Vgroup[(unsigned int)V[i].mark]++;
			if ((sum >= q)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
			{
				imax = i;			
			}
		}
		if (V[i].mark >= 0) if (Vgroup[(unsigned int)V[i].mark] >= 0) sum = sum + V[i].opening;
		if (V[i].opening > 0)
		{
			if (V[i].mark >= 0) Vgroup[(unsigned int)V[i].mark]--;
			if ((sum > q)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
			{
				q = sum;
				imin = i;
			}
		}
	}
	if ((imax == -1)||(imin == (int)(V.size()))) x = interval();
	else x = interval(V[imin].val, V[imax].val);
}

// Group number in ygroup should be from 0 to nbgroups-1.
inline void C_acc_group_opt(box& x, vector<box>& yj, vector<int>& ygroup, int nbgroups)
{
	vector<box> y(yj.size());
	for (unsigned int j = 0; j < y.size(); j++)
	{
		y[j] = x&yj[j];
	}
	vector<interval> yi(y.size());
	for (int i = 1; i <= x.dim; i++)
	{
		for (unsigned int j = 0; j < y.size(); j++)
			yi[j] = y[j][i];
		C_acc_group_opt(x[i], yi, ygroup, nbgroups);
	}
}
#pragma endregion
#pragma region C_ACC_MU
// Characteristic function associated with the interval z.
inline int dzeta(double x, interval& z)
{
	if (z.isEmpty) return 0;
	if ((z.inf <= x)&&(x <= z.sup)) return 1; else return 0;
}

// Characteristic function associated with the box z.
inline int dzeta_box(box& x, box& z)
{
	if (In(x,z) != ifalse) return 1; else return 0;
}

inline double mu_q_in_mark_box(box& X, vector<box>& Z, void* pData)
{
	vector<double> zmark = *((vector<double>*)pData);
	unsigned int m = Z.size();
	unsigned int k = 0;
	double res = 0;

	for (k = 0; k < m; k++)
	{
		res = res+zmark[k]*dzeta_box(X, Z[k]);
	}

	return res;
}

inline double mu_q_in_mark(double x, vector<interval>& z, void* pData, int curdim)
{
	vector<double> zmark = *((vector<double>*)pData);
	unsigned int m = z.size();
	unsigned int k = 0;
	double res = 0;

	UNREFERENCED_PARAMETER(curdim);

	for (k = 0; k < m; k++)
	{
		// res = res+zmark[k]*dzeta(x, z[k]);
		if ((!z[k].isEmpty)&&(z[k].inf <= x)&&(x <= z[k].sup)) res = res+zmark[k];
	}

	return res;
}

inline double mu_q_in_box(box& X, vector<box>& Z, void* pData)
{
	unsigned int m = Z.size();
	unsigned int k = 0;
	double res = 0;

	UNREFERENCED_PARAMETER(pData);

	for (k = 0; k < m; k++)
	{
		res = res+dzeta_box(X, Z[k]);
	}
	res = res/m;

	return res;
}

inline double mu_q_in(double x, vector<interval>& z, void* pData, int curdim)
{
	unsigned int m = z.size();
	unsigned int k = 0;
	double res = 0;

	UNREFERENCED_PARAMETER(pData);
	UNREFERENCED_PARAMETER(curdim);

	for (k = 0; k < m; k++)
	{
		// res = res+dzeta(x, z[k]);
		if ((!z[k].isEmpty)&&(z[k].inf <= x)&&(x <= z[k].sup)) res = res+1;
	}
	res = res/m;

	return res;
}

inline void C_mu(interval& x, vector<interval>& y, double (*mu)(double, vector<interval>&, void*, int), double mu0, void* pData, int curdim)
{
	vector<bound> V;
	if (x.isEmpty) return;
	if (mu0 <= 0) return;
	for (unsigned int i = 0; i < y.size(); i++)
	{
		if (!y[i].isEmpty)
		{
			V.push_back(bound(y[i].inf, 1, 0));
			V.push_back(bound(y[i].sup, -1, 0));
		}
	}
	if (V.size() == 0)
	{
		x = interval();
		return;
	}
	V.push_back(bound(x.inf, 1, 0));
	V.push_back(bound(x.sup, -1, 0));
	sort(V.begin(), V.end());
	double mu_res = 0;
	int imin = (int)(V.size());
	int imax = -1;
	for (unsigned int i = 0; i < V.size(); i++)
	{
		mu_res = mu(V[i].val, y, pData, curdim);
		if ((mu_res >= mu0)&&(V[i].opening > 0)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
		{
			imin = min(imin, (int)i);
		}
		if ((mu_res >= mu0)&&(V[i].opening < 0)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
		{
			imax = i;
		}
	}
	if ((imax == -1)||(imin == (int)(V.size()))) x = interval();
	else x = interval(V[imin].val, V[imax].val);
}

inline void C_mu(box& x, vector<box>& yj, double (*mu)(double, vector<interval>&, void*, int), double mu0, void* pData)
{
	vector<box> y(yj.size());
	for (unsigned int j = 0; j < y.size(); j++)
	{
		y[j] = x&yj[j];
	}
	vector<interval> yi(y.size());
	for (int i = 1; i <= x.dim; i++)
	{
		for (unsigned int j = 0; j < y.size(); j++)
			yi[j] = y[j][i];
		C_mu(x[i], yi, mu, mu0, pData, i);
	}
}

inline void C_acc_mu(interval& x, vector<interval>& y, double (*mu)(double, vector<interval>&, void*, int), void* pData, int curdim)
{
	vector<bound> V;
	if (x.isEmpty) return;
	for (unsigned int i = 0; i < y.size(); i++)
	{
		if (!y[i].isEmpty)
		{
			V.push_back(bound(y[i].inf, 1, 0));
			V.push_back(bound(y[i].sup, -1, 0));
		}
	}
	if (V.size() == 0)
	{
		x = interval();
		return;
	}
	V.push_back(bound(x.inf, 1, 0));
	V.push_back(bound(x.sup, -1, 0));
	sort(V.begin(), V.end());
	double mu0 = -1;
	double mu_res = 0;
	int imin = (int)(V.size());
	int imax = -1;
	for (unsigned int i = 0; i < V.size(); i++)
	{
		mu_res = mu(V[i].val, y, pData, curdim);
		if ((mu_res > mu0)&&(V[i].opening > 0)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
		{
			mu0 = mu_res;
			imin = i;
		}
		if ((mu_res >= mu0)&&(V[i].opening < 0)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
		{
			imax = i;
		}
	}
	if ((imax == -1)||(imin == (int)(V.size()))) x = interval();
	else x = interval(V[imin].val, V[imax].val);
}

inline void C_acc_mu(box& x, vector<box>& yj, double (*mu)(double, vector<interval>&, void*, int), void* pData)
{
	vector<box> y(yj.size());
	for (unsigned int j = 0; j < y.size(); j++)
	{
		y[j] = x&yj[j];
	}
	vector<interval> yi(y.size());
	for (int i = 1; i <= x.dim; i++)
	{
		for (unsigned int j = 0; j < y.size(); j++)
			yi[j] = y[j][i];
		C_acc_mu(x[i], yi, mu, pData, i);
	}
}
#pragma endregion
#pragma region C_ACC_MARK
// qmark should be >= 0.
inline void C_q_in_mark(interval& x, double qmark, vector<interval>& y, vector<double>& ymark)
{
	vector<bound> V;
	if (x.isEmpty) return;
	if (qmark <= 0) return;
	for (unsigned int i = 0; i < y.size(); i++)
	{
		if (!y[i].isEmpty)
		{
			V.push_back(bound(y[i].inf, 1, ymark[i]));
			V.push_back(bound(y[i].sup, -1, ymark[i]));
		}
	}
	if (V.size() == 0)
	{
		x = interval();
		return;
	}
	V.push_back(bound(x.inf, 1, 0));
	V.push_back(bound(x.sup, -1, 0));
	sort(V.begin(), V.end());
	double sum = 0;
	int imin = (int)(V.size());
	int imax = -1;
	for (unsigned int i = 0; i < V.size(); i++)
	{
		if ((sum >= qmark)&&(V[i].opening < 0)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
		{
			imax = i;
		}
		sum = sum + V[i].opening*V[i].mark;
		if ((sum >= qmark)&&(V[i].opening > 0)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
		{
			imin = min(imin, (int)i);
		}
	}
	if ((imax == -1)||(imin == (int)(V.size()))) x = interval();
	else x = interval(V[imin].val, V[imax].val);
}

inline void C_q_in_mark(box& x, double qmark, vector<box>& yj, vector<double>& ymark)
{
	vector<box> y(yj.size());
	for (unsigned int j = 0; j < y.size(); j++)
	{
		y[j] = x&yj[j];
	}
	vector<interval> yi(y.size());
	for (int i = 1; i <= x.dim; i++)
	{
		for (unsigned int j = 0; j < y.size(); j++)
			yi[j] = y[j][i];
		C_q_in_mark(x[i], qmark, yi, ymark);
	}
}

inline void C_acc_mark(interval& x, vector<interval>& y, vector<double>& ymark)
{
	vector<bound> V;
	if (x.isEmpty) return;
	for (unsigned int i = 0; i < y.size(); i++)
	{
		if (!y[i].isEmpty)
		{
			V.push_back(bound(y[i].inf, 1, ymark[i]));
			V.push_back(bound(y[i].sup, -1, ymark[i]));
		}
	}
	if (V.size() == 0)
	{
		x = interval();
		return;
	}
	V.push_back(bound(x.inf, 1, 0));
	V.push_back(bound(x.sup, -1, 0));
	sort(V.begin(), V.end());
	double qmark = -oo;
	double sum = 0;
	int imin = (int)(V.size());
	int imax = -1;
	for (unsigned int i = 0; i < V.size(); i++)
	{
		if ((sum >= qmark)&&(V[i].opening < 0)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
		{
			imax = i;
		}
		sum = sum + V[i].opening*V[i].mark;
		if ((sum > qmark)&&(V[i].opening > 0)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
		{
			qmark = sum;
			imin = i;
		}
	}
	if ((imax == -1)||(imin == (int)(V.size()))) x = interval();
	else x = interval(V[imin].val, V[imax].val);
}

inline void C_acc_mark(box& x, vector<box>& yj, vector<double>& ymark)
{
	vector<box> y(yj.size());
	for (unsigned int j = 0; j < y.size(); j++)
	{
		y[j] = x&yj[j];
	}
	vector<interval> yi(y.size());
	for (int i = 1; i <= x.dim; i++)
	{
		for (unsigned int j = 0; j < y.size(); j++)
			yi[j] = y[j][i];
		C_acc_mark(x[i], yi, ymark);
	}
}

// qmark should be >= 0.
inline void C_q_in_mark(interval& x, int qmark, vector<interval>& y, vector<int>& ymark)
{
	vector<bound> V;
	if (x.isEmpty) return;
	if (qmark <= 0) return;
	for (unsigned int i = 0; i < y.size(); i++)
	{
		if (!y[i].isEmpty)
		{
			V.push_back(bound(y[i].inf, 1, ymark[i]));
			V.push_back(bound(y[i].sup, -1, ymark[i]));
		}
	}
	if (V.size() == 0)
	{
		x = interval();
		return;
	}
	V.push_back(bound(x.inf, 1, 0));
	V.push_back(bound(x.sup, -1, 0));
	sort(V.begin(), V.end());
	int sum = 0;
	int imin = (int)(V.size());
	int imax = -1;
	for (unsigned int i = 0; i < V.size(); i++)
	{
		if ((sum >= qmark)&&(V[i].opening < 0)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
		{
			imax = i;
		}
		sum = sum + V[i].opening*(int)V[i].mark;
		if ((sum >= qmark)&&(V[i].opening > 0)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
		{
			imin = min(imin, (int)i);
		}
	}
	if ((imax == -1)||(imin == (int)(V.size()))) x = interval();
	else x = interval(V[imin].val, V[imax].val);
}

inline void C_q_in_mark(box& x, int qmark, vector<box>& yj, vector<int>& ymark)
{
	vector<box> y(yj.size());
	for (unsigned int j = 0; j < y.size(); j++)
	{
		y[j] = x&yj[j];
	}
	vector<interval> yi(y.size());
	for (int i = 1; i <= x.dim; i++)
	{
		for (unsigned int j = 0; j < y.size(); j++)
			yi[j] = y[j][i];
		C_q_in_mark(x[i], qmark, yi, ymark);
	}
}

inline void C_acc_mark(interval& x, vector<interval>& y, vector<int>& ymark)
{
	int qmark_max = std::accumulate(ymark.begin(), ymark.end(), 0);

	for (unsigned int qmarki = qmark_max; qmarki >= 1; qmarki--)
	{
		interval xi = x;
		C_q_in_mark(xi, qmarki, y, ymark);
		if (!xi.isEmpty) 
		{
			x = xi;
			return;
		}
	}
}

inline void C_acc_mark(box& x, vector<box>& yj, vector<int>& ymark)
{
	vector<box> y(yj.size());
	for (unsigned int j = 0; j < y.size(); j++)
	{
		y[j] = x&yj[j];
	}
	vector<interval> yi(y.size());
	for (int i = 1; i <= x.dim; i++)
	{
		for (unsigned int j = 0; j < y.size(); j++)
			yi[j] = y[j][i];
		C_acc_mark(x[i], yi, ymark);
	}
}
#pragma endregion
#pragma region C_ACC
inline void C_acc(interval& x, vector<interval>& y)
{
	for (unsigned int qi = y.size(); qi >= 1; qi--)
	{
		interval xi = x;
		C_q_in(xi, qi, y);
		if (!xi.isEmpty) 
		{
			x = xi;
			return;
		}
	}
}

inline void C_acc(box& x, vector<box>& yj)
{
	vector<box> y(yj.size());
	for (unsigned int j = 0; j < y.size(); j++)
	{
		y[j] = x&yj[j];
	}
	vector<interval> yi(y.size());
	for (int i = 1; i <= x.dim; i++)
	{
		for (unsigned int j = 0; j < y.size(); j++)
			yi[j] = y[j][i];
		C_acc(x[i], yi);
	}
}

inline void C_acc_alternate(box& x, vector<box>& yj)
{
	for (unsigned int qi = yj.size(); qi >= 1; qi--)
	{
		box xi = x;
		C_q_in(xi, qi, yj);
		if (!xi.IsEmpty()) 
		{
			x = xi;
			return;
		}
	}
}

inline void C_acc_opt(interval& x, vector<interval>& y)
{
	vector<bound> V;
	if (x.isEmpty) return;
	for (unsigned int i = 0; i < y.size(); i++)
	{
		if (!y[i].isEmpty)
		{
			V.push_back(bound(y[i].inf, 1, 1));
			V.push_back(bound(y[i].sup, -1, 1));
		}
	}
	if (V.size() == 0)
	{
		x = interval();
		return;
	}
	V.push_back(bound(x.inf, 1, 0));
	V.push_back(bound(x.sup, -1, 0));
	sort(V.begin(), V.end());
	int q = -1;
	int sum = 0;
	int imin = (int)(V.size());
	int imax = -1;
	for (unsigned int i = 0; i < V.size(); i++)
	{
		if ((sum >= q)&&(V[i].opening < 0)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
		{
			imax = i;
		}
		sum = sum + V[i].opening*(int)V[i].mark;
		if ((sum > q)&&(V[i].opening > 0)&&(V[i].val >= x.inf)&&(V[i].val <= x.sup))
		{
			q = sum;
			imin = i;
		}
	}
	if ((imax == -1)||(imin == (int)(V.size()))) x = interval();
	else x = interval(V[imin].val, V[imax].val);
}

inline void C_acc_opt(box& x, vector<box>& yj)
{
	vector<box> y(yj.size());
	for (unsigned int j = 0; j < y.size(); j++)
	{
		y[j] = x&yj[j];
	}
	vector<interval> yi(y.size());
	for (int i = 1; i <= x.dim; i++)
	{
		for (unsigned int j = 0; j < y.size(); j++)
			yi[j] = y[j][i];
		C_acc_opt(x[i], yi);
	}
}
#pragma endregion
#pragma region SIVIA
inline void BoxTranslate(box &Src, box &Dest, interval distance, interval angle, int dir)
{
	box P = box(2);
	interval xvect = distance*Cos(angle);
	interval yvect = distance*Sin(angle);

	if (dir == 1) 
	{
		P[1] = Src[1]+xvect;
		P[2] = Src[2]+yvect;
		Dest = Inter(Dest,P);
	}
	else
	{
		P[1] = Dest[1]-xvect;
		P[2] = Dest[2]-yvect;
		Src = Inter(Src,P);
	}
}

//// Not tested nor finished...
//void CNotInSegment(interval& mx, interval& my, double ax, double ay, double bx, double by)
//{      
//	// contracte relativement à la contrainte : "m n'appartient pas au segment [a,b]"
//	Cnotin(mx, Union(interval(ax, ax), interval(bx, bx)));
//	Cnotin(my, Union(interval(ay, ay), interval(by, by)));
//	interval ma_x = ax - mx;
//	interval ma_y = ay - my;
//	interval ma_x1 = ma_x;
//	interval ma_y1 = ma_y;
//	interval ma_x2 = ma_x;
//	interval ma_y2 = ma_y;
//	double ab_x = bx - ax;
//	double ab_y = by - ay;
//	interval z1 = interval(-oo, 0);
//	interval z2 = interval(0, oo);
//	CDet(z1, ab_x, ab_y, ma_x1, ma_y1, -1);
//	CDet(z2, ab_x, ab_y, ma_x2, ma_y2, -1);
//	ma_x = Inter(ma_x,Union(ma_x1,ma_x2));
//	ma_y = Inter(ma_y,Union(ma_y1,ma_y2));
//	Cmoins(ma_y, ay, my, -1);
//	Cmoins(ma_x, ax, mx, -1);
//	if ((mx.isEmpty) || (my.isEmpty)) { mx.isEmpty = true; my.isEmpty = true; };
//}

inline void Contract(box& P, deque< vector<interval> >& d_all_mes_vector, deque<double>& alpha_mes_vector, 
					 interval& thetahat, 
					 double& d_max_err, double& alpha_max_err, int& sdir, interval& alphashat, 
					 vector<double>& walls_xa, vector<double>& walls_ya, vector<double>& walls_xb, vector<double>& walls_yb, 
					 vector<double>& circles_x, vector<double>& circles_y, vector<double>& circles_r)
{
	int i = 0, j = 0;
	vector<box> Pi;
	vector<int> Pigroup;

	Pi.clear();
	Pigroup.clear();
	for (i = 0; i < (int)alpha_mes_vector.size(); i++)
	{
		vector<box> Pj;
		Pj.clear();
		interval alphahat = interval(alpha_mes_vector[i]-alpha_max_err,alpha_mes_vector[i]+alpha_max_err);
		for (j = 0; j < (int)d_all_mes_vector[i].size(); j++)
		{
			interval dhat = d_all_mes_vector[i][j]+interval(-d_max_err,+d_max_err);
			//
			// Activate only if using getfirst or getlast...
			// If no obstacle found...
			//if ((Center(d_all_mes_vector[i][j]) < 0)||(Center(d_all_mes_vector[i][j]) > rangescale)||(d_all_mes_vector[i][j].isEmpty)) 
			//{
			//	dhat = interval(rangescale-d_max_err,oo);
			//	printf("Bad dhat\n");
			//}
			//

			box Pxyj = P;
			box Pt = box(interval(-oo,oo),interval(-oo,oo)); 
			BoxTranslate(Pxyj, Pt, dhat, sdir*alphahat+alphashat+thetahat, 1);
			interval xt = Pt[1];
			interval yt = Pt[2];
			CPointInSegmentsOrCircles(xt,yt,walls_xa,walls_ya,walls_xb,walls_yb,circles_x,circles_y,circles_r);
			if ((!xt.isEmpty)&&(!yt.isEmpty))
			{
				Pt = box(xt,yt);
				BoxTranslate(Pxyj, Pt, dhat, sdir*alphahat+alphashat+thetahat, -1);
				//Pj.push_back(Pxyj);
				Pi.push_back(Pxyj);
				Pigroup.push_back(i);
			}
		}
		//box Pxy = Union(Pj);
		//if (!Pxy.IsEmpty())
		//{
		//	Pi.push_back(Pxy);
		//}
	}
	if (Pi.size() > 0)
	{
		//C_q_in_group(P, alpha_mes_vector.size()-nb_outliers, Pi, Pigroup, alpha_mes_vector.size());
		//C_acc_group(P, Pi, Pigroup, alpha_mes_vector.size());
		//C_q_in(P, Pi.size()-nb_outliers, Pi);
		//C_q_in(P, alpha_mes_vector.size()-nb_outliers, Pi);
		C_acc_opt(P, Pi);
		C_acc_opt(P, Pi);
	}
}

inline void Contract_dyn(box& P, deque< vector<interval> >& d_all_mes_vector, deque<double>& alpha_mes_vector, 
						 deque<double>& t_history_vector, deque<interval>& xhat_history_vector, deque<interval>& yhat_history_vector, 
						 deque<interval>& thetahat_history_vector, deque<interval>& vxyhat_history_vector, 
						 double& d_max_err, double& alpha_max_err, int& sdir, interval& alphashat, 
						 vector<double>& walls_xa, vector<double>& walls_ya, vector<double>& walls_xb, vector<double>& walls_yb, 
						 vector<double>& circles_x, vector<double>& circles_y, vector<double>& circles_r)
{
	int i = 0, j = 0, k = 0;
	vector<box> Pi;
	vector<int> Pigroup;

	Pi.clear();
	Pigroup.clear();
	for (i = 0; i < (int)alpha_mes_vector.size(); i++)
	{
		vector<box> Pj;
		Pj.clear();
		interval alphahat = interval(alpha_mes_vector[i]-alpha_max_err,alpha_mes_vector[i]+alpha_max_err);
		for (j = 0; j < (int)d_all_mes_vector[i].size(); j++)
		{
			interval dhat = d_all_mes_vector[i][j]+interval(-d_max_err,+d_max_err);
			//
			// Activate only if using getfirst or getlast...
			// If no obstacle found...
			//if ((Center(d_all_mes_vector[i][j]) < 0)||(Center(d_all_mes_vector[i][j]) > rangescale)||(d_all_mes_vector[i][j].isEmpty)) 
			//{
			//	dhat = interval(rangescale-d_max_err,oo);
			//	printf("Bad dhat\n");
			//}
			//

			box Pxyj = box(xhat_history_vector[i],yhat_history_vector[i]);
			box Pt = box(interval(-oo,oo),interval(-oo,oo)); 
			BoxTranslate(Pxyj, Pt, dhat, sdir*alphahat+alphashat+thetahat_history_vector[i], 1);
			interval xt = Pt[1];
			interval yt = Pt[2];
			CPointInSegmentsOrCircles(xt,yt,walls_xa,walls_ya,walls_xb,walls_yb,circles_x,circles_y,circles_r);
			if ((!xt.isEmpty)&&(!yt.isEmpty))
			{
				Pt = box(xt,yt);
				BoxTranslate(Pxyj, Pt, dhat, sdir*alphahat+alphashat+thetahat_history_vector[i], -1);

				// Propagate forward to current position...
				interval xk = xhat_history_vector[i] & Pxyj[1];
				interval yk = yhat_history_vector[i] & Pxyj[2];
				for (k = i; k < (int)alpha_mes_vector.size()-1; k++)
				{
					double dt = t_history_vector[k+1]-t_history_vector[k];
					xk = xhat_history_vector[k+1] & (xk+dt*vxyhat_history_vector[k]*Cos(thetahat_history_vector[k]));
					yk = yhat_history_vector[k+1] & (yk+dt*vxyhat_history_vector[k]*Sin(thetahat_history_vector[k]));
					if ((xk.isEmpty)||(yk.isEmpty)) break;
				}
				Pxyj = box(xk,yk);

				if (!Pxyj.IsEmpty())
				{
					//Pj.push_back(Pxyj);
					Pi.push_back(Pxyj);
					Pigroup.push_back(i);
				}
			}
		}
		//box Pxy = Union(Pj);
		//if (!Pxy.IsEmpty())
		//{
		//	Pi.push_back(Pxy);
		//}
	}
	if (Pi.size() > 0)
	{
		//C_q_in_group(P, alpha_mes_vector.size()-nb_outliers, Pi, Pigroup, alpha_mes_vector.size());
		//C_acc_group(P, Pi, Pigroup, alpha_mes_vector.size());
		//C_q_in(P, Pi.size()-nb_outliers, Pi);
		//C_q_in(P, alpha_mes_vector.size()-nb_outliers, Pi);
		C_acc_opt(P, Pi);
		C_acc_opt(P, Pi);
	}
}

inline box SIVIA(box P0, deque< vector<interval> >& d_all_mes_vector, deque<double>& alpha_mes_vector, 
				 interval& thetahat, 
				 double& d_max_err, double& alpha_max_err, int& sdir, interval& alphashat, 
				 vector<double>& walls_xa, vector<double>& walls_ya, vector<double>& walls_xb, vector<double>& walls_yb, 
				 vector<double>& circles_x, vector<double>& circles_y, vector<double>& circles_r)
{   
	list<box> L;
	vector<box> Result;
	box P, P1, P2;
	int nb_bisections = 0;

	L.push_back(P0);
	while (!L.empty())
	{ 
		P = L.front();
		L.pop_front();

		Contract(P, d_all_mes_vector, alpha_mes_vector, 
			thetahat, 
			d_max_err, alpha_max_err, sdir, alphashat, 
			walls_xa, walls_ya, walls_xb, walls_yb, 
			circles_x, circles_y, circles_r);

		if (!P.IsEmpty())
		{  
			//if (P.Width() < 0.1)
			if (nb_bisections > 10)
			{

				/*				//
				COORDSYSTEM2IMG csMap2FullImg;

				InitCS2ImgEx(&csMap2FullImg, &csMap, imgwidth, imgheight, BEST_RATIO_COORDSYSTEM2IMG);

				cvRectangle(img, 
				cvPoint(
				XCS2JImg(&csMap2FullImg, Center(P[1])-0.5*Width(P[1])), 
				YCS2IImg(&csMap2FullImg, Center(P[2])-0.5*Width(P[2]))), 
				cvPoint(
				XCS2JImg(&csMap2FullImg, Center(P[1])+0.5*Width(P[1])), 
				YCS2IImg(&csMap2FullImg, Center(P[2])+0.5*Width(P[2]))), 
				CV_RGB(0, 128, 128), CV_FILLED, 8, 0);
				cvRectangle(img, 
				cvPoint(
				XCS2JImg(&csMap2FullImg, Center(P[1])-0.5*Width(P[1])), 
				YCS2IImg(&csMap2FullImg, Center(P[2])-0.5*Width(P[2]))), 
				cvPoint(
				XCS2JImg(&csMap2FullImg, Center(P[1])+0.5*Width(P[1])), 
				YCS2IImg(&csMap2FullImg, Center(P[2])+0.5*Width(P[2]))), 
				CV_RGB(0, 255, 255), 1, 8, 0);
				*/				//

				Result.push_back(P);
			}
			else  
			{   
				Bisect(P, P1, P2);
				L.push_back(P1);
				L.push_back(P2);
				nb_bisections++;
			}
		}
	}

	P = Union(Result);
	//printf("nb_bisections = %d\n", nb_bisections);

	return P;
}

inline box SIVIA_dyn(box P0, deque< vector<interval> >& d_all_mes_vector, deque<double>& alpha_mes_vector, 
						 deque<double>& t_history_vector, deque<interval>& xhat_history_vector, deque<interval>& yhat_history_vector, 
						 deque<interval>& thetahat_history_vector, deque<interval>& vxyhat_history_vector, 
						 double& d_max_err, double& alpha_max_err, int& sdir, interval& alphashat, 
						 vector<double>& walls_xa, vector<double>& walls_ya, vector<double>& walls_xb, vector<double>& walls_yb, 
						 vector<double>& circles_x, vector<double>& circles_y, vector<double>& circles_r)
{   
	list<box> L;
	vector<box> Result;
	box P, P1, P2;
	int nb_bisections = 0;

	L.push_back(P0);
	while (!L.empty())
	{ 
		P = L.front();
		L.pop_front();

		Contract_dyn(P, d_all_mes_vector, alpha_mes_vector, 
			t_history_vector, xhat_history_vector, yhat_history_vector, 
			thetahat_history_vector, vxyhat_history_vector,
			d_max_err, alpha_max_err, sdir, alphashat, 
			walls_xa, walls_ya, walls_xb, walls_yb, 
			circles_x, circles_y, circles_r);

		if (!P.IsEmpty())
		{  
			//if (P.Width() < 0.1)
			if (nb_bisections > 10)
			{

				/*				//
				COORDSYSTEM2IMG csMap2FullImg;

				InitCS2ImgEx(&csMap2FullImg, &csMap, imgwidth, imgheight, BEST_RATIO_COORDSYSTEM2IMG);

				cvRectangle(img, 
				cvPoint(
				XCS2JImg(&csMap2FullImg, Center(P[1])-0.5*Width(P[1])), 
				YCS2IImg(&csMap2FullImg, Center(P[2])-0.5*Width(P[2]))), 
				cvPoint(
				XCS2JImg(&csMap2FullImg, Center(P[1])+0.5*Width(P[1])), 
				YCS2IImg(&csMap2FullImg, Center(P[2])+0.5*Width(P[2]))), 
				CV_RGB(0, 128, 128), CV_FILLED, 8, 0);
				cvRectangle(img, 
				cvPoint(
				XCS2JImg(&csMap2FullImg, Center(P[1])-0.5*Width(P[1])), 
				YCS2IImg(&csMap2FullImg, Center(P[2])-0.5*Width(P[2]))), 
				cvPoint(
				XCS2JImg(&csMap2FullImg, Center(P[1])+0.5*Width(P[1])), 
				YCS2IImg(&csMap2FullImg, Center(P[2])+0.5*Width(P[2]))), 
				CV_RGB(0, 255, 255), 1, 8, 0);
				*/				//

				Result.push_back(P);
			}
			else  
			{   
				Bisect(P, P1, P2);
				L.push_back(P1);
				L.push_back(P2);
				nb_bisections++;
			}
		}
	}

	P = Union(Result);
	//printf("nb_bisections = %d\n", nb_bisections);

	return P;
}
#pragma endregion
#pragma region SONAR_DETECTIONS
/*
The 'Dynamic Range Control' is a surface display function which has 2 parameters (which are sent
to the Sonar in the 'mtHeadCommand').
These parameters are;
ADLow : This sets the Lower boundary of the sampling window. This value can be increased to
make the Sonar display less sensitive and filter out background and receiver self noise.
ADSpan : This sets the width of the sampling window and therefore acts as a Contrast control.
The ADLow and ADSpan are applied in different areas depending on what A/D sampling mode is
configured (see 'Dbytes' below for more information);
4-bit mode : The 4-bit data Bins are digitized inside the Sonar using the A/D sampling window set
by ADLow and ADSpan. On receipt of the 'mtHeadData' replies, the 4-bit data bins are therefore
already mapped to the A/D sampling window giving 16 intensity levels (0..15) which are ready for
plotting in surface software.
8-bit mode : The 8-bit data Bins are digitized and mapped to the full 0..80dB range of the receiver.
On receipt of the 'mtHeadData' replies, the ADLow and ADSpan then need to be applied in surface
software to set the A/D sampling window to which the 8-bit data Bins will map onto.
*/
inline unsigned char DynamicRangeControl(unsigned char Bin, unsigned char ADLow, unsigned char ADSpan, int adc8on)
{
	// The sonar receiver has an 80dB dynamic range, and signal 
	// levels are processed internally by the sonar head such that
	// 0 .. 80dB = 0 .. 255
	// If adc8on in HdCtrl = 0, then the 80dB receiver signal is mapped to 4-bit, 16 level reply data
	// values, to a display dynamic range defined by ADSpan and ADLow such that:
	// ADSpan = 255 * Span(dB) / 80
	// ADLow = 255 * Low(dB) / 80
	// For example, ADSpan = 38 (12dB), ADLow = 40 (13dB)
	// 4-bit Data Values 0..15 = Signal Amplitudes 0 = 13dB, 15 = 25dB
	// ADSpan = 77 (24 dB) and ADLow = 40 (13dB) are typical values
	// If adc8on = 1 then the full 8-bit, 80dB dynamic range data bin amplitudes are returned to
	// the user:
	// 8-bit data Values 0..255 = Signal Amplitudes 0 = 0dB, 255 = 80dB

	if (adc8on)
	{
		if (Bin <= ADLow) return 0;
		else if (Bin >= ADLow+ADSpan) return 255;
		else return (unsigned char)((double)(Bin-ADLow)*255.0/(double)ADSpan);
	}
	else
	{
		return (unsigned char)(Bin);
	}
}

inline unsigned char DynamicRangeControlInv(unsigned char Bin, unsigned char ADLow, unsigned char ADSpan, int adc8on)
{
	if (adc8on)
	{
		return (unsigned char)((double)(Bin+ADLow)*(double)ADSpan/255.0);
	}
	else
	{
		return (unsigned char)(Bin);
	}
}

/*
Return the distance to the first obstacle from a sonar scanline.
The function does not check any argument.

unsigned char* scanline : (IN) Pointer to the sonar scanline to process.
unsigned char threshold : (IN) Threshold that indicates an obstacle (from 0 to 255).
double minDist : (IN) Distance from which we begin the search of the first 
obstacle (in m).
double maxDist : (IN) Distance to which we stop the search of the first 
obstacle (in m).
int NBins : (IN) Number of bins per scanline.
int RangeScale : (IN) Sonar range scale.
double* pDist : (INOUT) Valid pointer that will receive the distance to the 
first obstacle in m. Return -1 if all the bins are under the threshold (i.e. 
no obstacle).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int GetFirstObstacleDist(unsigned char* scanline, unsigned char threshold, double minDist, double maxDist, 
								int NBins, int RangeScale, double* pDist)
{
	double r0 = (double)NBins/(double)RangeScale;
	double r1 = (double)RangeScale/(double)NBins;
	int minBin = max(0, (int)(minDist*r0));
	int maxBin = min(NBins-1, (int)(maxDist*r0));
	int i = 0;

	i = minBin;
	while ((i <= maxBin) && (scanline[i] < threshold))
	{
		i++;
	}

	if (i > maxBin)
	{ 
		*pDist = -1;
	}
	else	
	{
		*pDist = (double)i*r1; // Convert in m.
	}

	return EXIT_SUCCESS;
}

inline int GetLastObstacleDist(unsigned char* scanline, unsigned char threshold, double minDist, double maxDist, 
							   int NBins, int RangeScale, double* pDist)
{
	double r0 = (double)NBins/(double)RangeScale;
	double r1 = (double)RangeScale/(double)NBins;
	int minBin = max(0, (int)(minDist*r0));
	int maxBin = min(NBins-1, (int)(maxDist*r0));
	int i = 0;

	i = maxBin;
	while ((i >= minBin) && (scanline[i] < threshold))
	{
		i--;
	}

	if (i < minBin)
	{ 
		*pDist = -1;
	}
	else	
	{
		*pDist = (double)i*r1; // Convert in m.
	}

	return EXIT_SUCCESS;
}

// threshold should be in [0..255].
inline int GetAllObstaclesDist(unsigned char* scanline, unsigned char threshold, double minDist, double maxDist, 
							   int NBins, int RangeScale, vector<interval>& distances)
{
	double r0 = (double)NBins/(double)RangeScale;
	double r1 = (double)RangeScale/(double)NBins;
	int minBin = max(0, (int)(minDist*r0));
	int maxBin = min(NBins-1, (int)(maxDist*r0));
	int i = 0;
	double d0 = -1, d1 = -1;

	d0 = 0;
	i = max(1, minBin);
	while (i <= maxBin)
	{
		if (scanline[i] >= threshold)
		{
			if (d0 < 0) 
			{
				d0 = (double)i*r1; // Convert in m.
			}
		}
		else
		{
			if (d0 >= 0) 
			{
				d1 = (double)(i-1)*r1; // Convert in m.
				distances.push_back(interval(d0, d1));
				d0 = -1;
				d1 = -1;
			}
		}
		i++;
	}

	if (d0 >= 0) 
	{
		distances.push_back(interval(d0, oo));
	}
	else
	{
		distances.push_back(interval(maxDist, oo));
	}

	return EXIT_SUCCESS;
}

// threshold should be in [0..255] with Dynamic Range Control.
inline int GetAllObstaclesDistDRC(unsigned char* scanline, unsigned char threshold, double minDist, double maxDist, 
								  int NBins, int RangeScale, unsigned char ADLow, unsigned char ADSpan, int adc8on, vector<interval>& distances)
{
	double r0 = (double)NBins/(double)RangeScale;
	double r1 = (double)RangeScale/(double)NBins;
	int minBin = max(0, (int)(minDist*r0));
	int maxBin = min(NBins-1, (int)(maxDist*r0));
	int i = 0;
	double d0 = -1, d1 = -1;

	d0 = 0;
	i = max(1, minBin);
	while (i <= maxBin)
	{
		if (DynamicRangeControl(scanline[i], ADLow, ADSpan, adc8on) >= threshold)
		{
			if (d0 < 0) 
			{
				d0 = (double)i*r1; // Convert in m.
			}
		}
		else
		{
			if (d0 >= 0) 
			{
				d1 = (double)(i-1)*r1; // Convert in m.
				distances.push_back(interval(d0, d1));
				d0 = -1;
				d1 = -1;
			}
		}
		i++;
	}

	if (d0 >= 0) 
	{
		distances.push_back(interval(d0, oo));
	}
	else
	{
		distances.push_back(interval(maxDist, oo));
	}

	return EXIT_SUCCESS;
}

// The search of obstacles only starts at minDist, using differentiation.
inline int GetAllObstaclesDistDiff(unsigned char* scanline, unsigned char absdiffthreshold, int diffstep, double minDist, double maxDist, 
								   int NBins, int RangeScale, vector<interval>& distances)
{
	double r0 = (double)NBins/(double)RangeScale;
	double r1 = (double)RangeScale/(double)NBins;
	int minBin = max(0, (int)(minDist*r0));
	int maxBin = min(NBins-1, (int)(maxDist*r0));
	int i = 0;
	double d0 = -1, d1 = -1;

	i = minBin;
	while (i <= maxBin-diffstep)
	{
		if (abs((int)(scanline[i+diffstep]-scanline[i])) >= (int)absdiffthreshold)
		{
			if (d0 < 0) 
			{
				d0 = (double)i*r1; // Convert in m.
			}
		}
		else
		{
			if (d0 >= 0) 
			{
				d1 = (double)(i-1)*r1; // Convert in m.
				distances.push_back(interval(d0, d1));
				d0 = -1;
				d1 = -1;
			}
		}
		i++;
	}

	if (d0 >= 0) 
	{
		distances.push_back(interval(d0, oo));
	}
	else
	{
		distances.push_back(interval(maxDist, oo));
	}

	return EXIT_SUCCESS;
}

// The search of obstacles only starts at minDist, using differentiation, with Dynamic Range Control.
inline int GetAllObstaclesDistDiffDRC(unsigned char* scanline, unsigned char absdiffthreshold, int diffstep, double minDist, double maxDist, 
									  int NBins, int RangeScale, unsigned char ADLow, unsigned char ADSpan, int adc8on, vector<interval>& distances)
{
	double r0 = (double)NBins/(double)RangeScale;
	double r1 = (double)RangeScale/(double)NBins;
	int minBin = max(0, (int)(minDist*r0));
	int maxBin = min(NBins-1, (int)(maxDist*r0));
	int i = 0;
	double d0 = -1, d1 = -1;

	i = minBin;
	while (i <= maxBin-diffstep)
	{
		if (abs((int)(DynamicRangeControl(scanline[i+diffstep], ADLow, ADSpan, adc8on)-DynamicRangeControl(scanline[i], ADLow, ADSpan, adc8on))) >= (int)absdiffthreshold)
		{
			if (d0 < 0) 
			{
				d0 = (double)i*r1; // Convert in m.
			}
		}
		else
		{
			if (d0 >= 0) 
			{
				d1 = (double)(i-1)*r1; // Convert in m.
				distances.push_back(interval(d0, d1));
				d0 = -1;
				d1 = -1;
			}
		}
		i++;
	}

	if (d0 >= 0) 
	{
		distances.push_back(interval(d0, oo));
	}
	else
	{
		distances.push_back(interval(maxDist, oo));
	}

	return EXIT_SUCCESS;
}

inline int GetAllObstaclesDist5(unsigned char* scanline, unsigned char threshold, int diffstep, double minDist, double maxDist, 
								int NBins, int RangeScale, unsigned char ADLow, unsigned char ADSpan, int adc8on, vector<interval>& distances)
{
	double r0 = (double)NBins/(double)RangeScale;
	double r1 = (double)RangeScale/(double)NBins;
	int minBin = max(0, (int)(minDist*r0));
	int maxBin = min(NBins-1, (int)(maxDist*r0));
	int i = 0;
	double d0 = -1, d1 = -1;
	unsigned char* scanlinethreshhold = NULL;

	scanlinethreshhold = (unsigned char*)malloc(NBins*sizeof(unsigned char));
	if (scanlinethreshhold == NULL)
	{
		return EXIT_FAILURE;
	}

	i = minBin;
	while (i <= maxBin)
	{
		if (DynamicRangeControl(scanline[i], ADLow, ADSpan, adc8on) >= threshold)
		{
			scanlinethreshhold[i] = 255;
		}
		else
		{
			scanlinethreshhold[i] = 0;
		}
		i++;
	}

	i = minBin;
	while (i <= maxBin-diffstep)
	{
		if (abs((int)(scanlinethreshhold[i+diffstep]-scanlinethreshhold[i])) >= 255)
		{
			if (d0 < 0) 
			{
				d0 = (double)i*r1; // Convert in m.
			}
		}
		else
		{
			if (d0 >= 0) 
			{
				d1 = (double)(i-1)*r1; // Convert in m.
				distances.push_back(interval(d0, d1));
				d0 = -1;
				d1 = -1;
			}
		}
		i++;
	}

	if (d0 >= 0) 
	{
		distances.push_back(interval(d0, oo));
	}
	else
	{
		distances.push_back(interval(maxDist, oo));
	}

	free(scanlinethreshhold);

	return EXIT_SUCCESS;
}
#pragma endregion
#endif // COMPUTATIONS_H
