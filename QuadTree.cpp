// QuadTree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <math.h>
#include <algorithm>
#include "drawing_c.h"

using namespace std;
#pragma warning(disable:4996)
enum Directions { SW, SE, NW, NE };
enum NodeColor { BLACK, WHITE, GRAY };
typedef struct point {
	double x, y;
} point;
typedef struct edge {
	point a, b;
	char name[8];
} edge;
typedef struct quad {
	struct quad* child[4];
	point A, B;
	vector<edge> edges;
	NodeColor rootColor = BLACK;
} quad;
enum InstructionTypes
{
	INIT_QUADTREE, DISPLAY,
	LIST_LINES,
	CREATE_LINE,
	LINE_SEARCH,
	INSERT,
	DELETE,
	DELETE_POINT,
	NEIGHBOR,
	KTH_NEIGHBOR,
	WINDOW,
	WINDOW_DISPLAY,
	FIND_POLYGON,
	BUILD_QUADTREE,
	ARCHIVE_QUADTREE,
	NONE
};
InstructionTypes GetInstructionType(string cmnd)
{
	if (cmnd.compare("INIT_QUADTREE") == 0)
		return INIT_QUADTREE;
	if (cmnd.compare("DISPLAY") == 0)
		return DISPLAY;
	if (cmnd.compare("LIST_LINES") == 0)
		return LIST_LINES;
	if (cmnd.compare("CREATE_LINE") == 0)
		return CREATE_LINE;
	if (cmnd.compare("LINE_SEARCH") == 0)
		return LINE_SEARCH;
	if (cmnd.compare("INSERT") == 0)
		return INSERT;
	if (cmnd.compare("DELETE") == 0)
		return DELETE;
	if (cmnd.compare("DELETE_POINT") == 0)
		return DELETE_POINT;
	if (cmnd.compare("NEIGHBOR") == 0)
		return NEIGHBOR;
	if (cmnd.compare("KTH_NEIGHBOR") == 0)
		return KTH_NEIGHBOR;
	if (cmnd.compare("WINDOW") == 0)
		return WINDOW;
	if (cmnd.compare("WINDOW_DISPLAY") == 0)
		return WINDOW_DISPLAY;
	if (cmnd.compare("FIND_POLYGON") == 0)
		return FIND_POLYGON;
	if (cmnd.compare("BUILD_QUADTREE") == 0)
		return BUILD_QUADTREE;
	if (cmnd.compare("ARCHIVE_QUADTREE") == 0)
		return ARCHIVE_QUADTREE;
	return NONE;
}
// parse an instruction to its parameters
vector<string> ParseInstruction(string inst)
{
	vector<string> res;
	string tmp = "";
	for (int i = 0; i < inst.length(); i++)
	{
		// delimetters
		if (inst[i] == '(' || inst[i] == ',' || inst[i] == ')')
		{
			if (tmp.length() > 0)
				res.push_back(tmp);
			tmp = "";
		}
		else
			tmp += inst[i];
	}
	return res;
}
// inserts the line in the vector and keeps it sorted
void InsertLine(vector<edge> * lines, edge e)
{
	int i = 0;
	for (; i < lines->size(); i++)
	{
		if (strcmp((*lines)[i].name, e.name) > 0)
			break;
	}
	(*lines).insert((*lines).begin() + i, e);
}

void init_quadtree(quad * q, int width)
{
	q = new quad();
	point a = { 0, 0 };
	point b = { 2 << width, 2 << width };
	q->A = a;
	q->B = b;
}
void ListLines(vector<edge> lines)
{
	vector<edge> vc(lines);
	for (int i = 0; i < vc.size(); i++)
	{
		printf("%s\n", vc[i].name);
	}

	// print results
}
edge Create_Line(string name, double ax, double ay, double bx, double by)
{
	point a;
	a.x = ax; a.y = ay;
	point b;
	b.x = bx; b.y = by;
	edge res;
	res.a = a;
	res.b = b;
	strcpy(res.name, name.c_str());
	return res;
}
// Given three colinear points p, q, r, the function checks if 
// point q lies on line segment 'pr' 
bool onSegment(point p, point q, point r)
{
	if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
		q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
		return true;

	return false;
}

// To find orientation of ordered triplet (p, q, r). 
// The function returns following values 
// 0 --> p, q and r are colinear 
// 1 --> Clockwise 
// 2 --> Counterclockwise 
int orientation(point p, point q, point r)
{
	// See https://www.geeksforgeeks.org/orientation-3-ordered-points/ 
	// for details of below formula. 
	int val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

	if (val == 0) return 0;  // colinear 

	return (val > 0) ? 1 : 2; // clock or counterclock wise 
}

// The main function that returns true if line segment 'p1q1' 
// and 'p2q2' intersect. 
bool doIntersect(point p1, point q1, point p2, point q2)
{
	// Find the four orientations needed for general and 
	// special cases 
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	// check for intersection on endpoint
	if (o1 * o2 * o3 * o4 == 0)
	{
		if ((p1.x == p2.x && p1.y == p2.y) ||
			(p1.x == q2.x && p1.y == q2.y) ||
			(q1.x == p2.x && q1.y == p2.y) ||
			(q1.x == q2.x && q1.y == q2.y))
			return false;
	}

	// General case 
	if (o1 != o2 && o3 != o4)
		return true;

	// Special Cases 
	// p1, q1 and p2 are colinear and p2 lies on segment p1q1 
	if (o1 == 0 && onSegment(p1, p2, q1)) return true;

	// p1, q1 and q2 are colinear and q2 lies on segment p1q1 
	if (o2 == 0 && onSegment(p1, q2, q1)) return true;

	// p2, q2 and p1 are colinear and p1 lies on segment p2q2 
	if (o3 == 0 && onSegment(p2, p1, q2)) return true;

	// p2, q2 and q1 are colinear and q1 lies on segment p2q2 
	if (o4 == 0 && onSegment(p2, q1, q2)) return true;

	return false; // Doesn't fall in any of the above cases 
}
// returns nonzero if e1 and e2 intersect
int line_intersect(edge e1, edge e2)
{
	return doIntersect(e1.a, e1.b, e2.a, e2.b);
}
// returns index of line in list
int line_ind(vector<edge> lines, string name)
{
	for (int i = 0; i < lines.size(); i++)
	{
		if (strcmp(lines[i].name, name.c_str()) == 0)
			return i;
	}
	return -1;
}
// returns true if e untersects window of q
int window_intersect(quad * q, edge e)
{
	return 0;
}
//  returns a list of intersected lines with line
vector<edge> line_search(quad * q, edge e)
{
	vector<edge> res;
	if (q->rootColor == GRAY)
	{
		// q has 4 children
		for (int i = 0; i < 4; i++)
		{
			if (window_intersect(q->child[i], e))
			{
				vector<edge> temp = line_search(q->child[i], e);
				res.insert(res.end(), temp.begin(), temp.end());
			}
		}
	}
	else
	{
		for (int i = 0; i < q->edges.size(); i++)
		{
			if (line_intersect(e, q->edges[i]))
				res.push_back(q->edges[i]);
		}
	}
	return res;
}
int isinvalid(quad * q)
{
	return 1;
}
// insert the new edge in q
void insert(quad * q, edge e)
{
	if (q->rootColor == GRAY)
	{
		for (int i = 0; i < 4; i++)
		{
			if (window_intersect(q->child[i], e))
			{
				insert(q->child[i], e);
			}
		}
	}
	else
	{
		q->edges.push_back(e);
		if (isinvalid(q))
		{
			q->rootColor = GRAY;
			for (int i = 0; i < 4; i++)
			{
				q->child[i] = new quad();

				q->child[i]->rootColor = BLACK;
				for (int j = 0; j < q->edges.size(); j++)
				{
					if (window_intersect(q->child[i], q->edges[j]))
					{
						insert(q->child[i], q->edges[j]);
					}
				}
			}
			point EWM1 = { (q->B.x - q->A.x) / 2 ,  q->A.y };
			point EWM2 = { (q->B.x - q->A.x) / 2 ,  (q->B.y - q->A.y) / 2 };
			point EWM3 = { (q->B.x - q->A.x) / 2 ,  q->B.y };

			point NSM1 = { q->A.x , (q->B.y - q->A.y) / 2 };
			point NSM2 = { (q->B.x - q->A.x) / 2 , (q->B.y - q->A.y) / 2 };
			point NSM3 = { q->B.x , (q->B.y - q->A.y) / 2 };


			q->child[SW]->A = NSM1; q->child[SW]->B = EWM3;
			q->child[SE]->A = EWM2; q->child[SE]->B = q->B;
			q->child[NW]->A = q->A; q->child[NW]->B = EWM2;
			q->child[NE]->A = EWM1; q->child[NE]->B = NSM3;

			q->edges.clear();
		}
	}
}
// print lines in the quad tree
void printQTlines(quad * q)
{
	for (int i = 0; i < q->edges.size(); i++)
	{
		DrawLine(q->edges[i].a.x, q->edges[i].a.y, q->edges[i].b.x, q->edges[i].b.y);
	}
	if (q->rootColor == BLACK)
	{
		return;
	}
	for (int i = 0; i < 4; i++)
	{
		printQTlines(q->child[i]);
	}
}
// prints dashed lines of the quad tree 
void printQT(quad * q)
{
	DrawLine(q->A.x, q->A.y, q->B.x, q->B.y);
	if (q->rootColor == BLACK)
	{
		// q is a leaf
		return;
	}
	for (int i = 0; i < 4; i++)
	{
		printQT(q->child[i]);
	}
}
int delete_line(quad * q, edge e)
{
	// search for line with name= name
	return 0;
}
string neighbor(quad * q, point p)
{
	return "";
}
string kth_neighbor(quad * q, point p)
{
	return "";
}
int window(quad * q, point a, point b)
{
	return 0;
}
void window_display(quad * q, point a, point b)
{

}
int find_polygon(quad * q, point p)
{
	return 0;
}
int main()
{
	// start of the program
	quad* q = NULL;
	// the list of lines sorted in alphanumeric order
	vector <edge> lines;

	// change cin to read from file --- for debugging and tests
	//std::ifstream in("input.txt");
	//std::streambuf* cinbuf = std::cin.rdbuf(); //save old buf
	//std::cin.rdbuf(in.rdbuf()); //redirect std::cin to in.txt!

	int ind;
	int Exit = 0;
	while (Exit == 0)
	{
		string instruction;
		cin >> instruction;
		if (instruction.length() == 0)
			break;
		vector<string> parts = ParseInstruction(instruction);
		int res; string tmp;
		vector<edge> linestmp;
		point p1, p2;
		switch (GetInstructionType(parts[0]))
		{
		case INIT_QUADTREE:
			res = stoi(parts[1]); res--;
			q = new quad();
			p1.x = 0; p1.y = 0;
			p2.x = (2 << res) - 1; p2.y = (2 << res) - 1;
			q->A = p1;
			q->B = p2;
			q->rootColor = BLACK;
			break;


		case LIST_LINES:
			ListLines(lines);
			break;

		case CREATE_LINE:
			edge e = Create_Line(parts[1], stod(parts[2]), stod(parts[3]), stod(parts[4]), stod(parts[5]));
			//add line to line list
			InsertLine(&lines, e);
			printf("LINE %s IS CREATED\n", parts[1].c_str());
			break;
		case DISPLAY:
			StartPicture(q->B.x + 1, q->B.y + 1);
			printQTlines(q);
			SetLineDash(1, 1);
			printQT(q);
			EndPicture();
			break;
		case LINE_SEARCH:
			ind = line_ind(lines, parts[1].c_str());
			linestmp = line_search(q, lines[ind]);
			if (linestmp.size() == 0)
				printf("%s DOES NOT INTERSECT ANY EXISTING LINE\n", parts[1].c_str());
			else
				printf("%s INTERSECTS %d LINE(S)\n", parts[1].c_str(), linestmp.size());
			break;
		case INSERT:
			ind = line_ind(lines, parts[1].c_str());
			linestmp = line_search(q, lines[ind]);
			if (linestmp.size() == 0)
			{
				// insert line in qtree
				insert(q, lines[ind]);

				printf("%s IS INSERTED\n", parts[1].c_str());
			}
			else
				printf("LINE %s INTERSECTS %d LINE(S)\n", parts[1].c_str(), res);
			break;
		case DELETE:
			ind = line_ind(lines, parts[1]);

			if (ind >= 0)
			{
				delete_line(q, lines[ind]);
				printf("%s DOES NOT EXIST\n", parts[1]);
			}
			else
				printf("%s IS DELETED\n", parts[1], res);
			break;
		case DELETE_POINT:
			p1.x = stod(parts[1]); p1.y = stod(parts[2]);
			res = 0;
			for (int i = 0; i < lines.size(); i++)
			{
				if ((lines[i].a.x == p1.x && lines[i].a.y == p1.y) ||
					(lines[i].b.x == p1.x && lines[i].b.y == p1.y))
				{
					delete_line(q, lines[i]);
					res++;
				}
			}
			if (res == 0)
				printf("%s LINE(S) DELETED\n", res);
			else
				printf("LINES DO NOT EXIST\n");
			break;
		case NEIGHBOR:
			p1.x = stod(parts[1]);p1.y = stod(parts[2]);

			tmp = neighbor(q, p1);
			if (tmp.length() == 0)
				printf("THE LINE DOES NOT EXIST\n");
			else
				printf("THE NEAREST NEIGHBOR IS %s\n", tmp);
			break;
		case KTH_NEIGHBOR:
			p1.x = stod(parts[1]); p1.y = stod(parts[2]);

			tmp = kth_neighbor(q, p1);
			if (tmp.length() == 0)
				printf("THE LINE DOES NOT EXIST\n");
			else
				printf("THE KTH NEAREST NEIGHBOR IS %s\n", tmp);
			break;
		case WINDOW:
			p1.x = stod(parts[1]); p2.y = stod(parts[2]);
			p2.x = stod(parts[3]); p2.y = stod(parts[4]);
			res = window(q, p1, p2);

			printf("%d LINE(S) IN THE WINDOW\n", res);
			break;
		case WINDOW_DISPLAY:
			p1.x = stod(parts[1]); p2.y = stod(parts[2]);
			p2.x = stod(parts[3]); p2.y = stod(parts[4]);
			window_display(q, p1, p2);
			break;
		case FIND_POLYGON:
			p1.x = stod(parts[1]); p2.y = stod(parts[2]);
			res = find_polygon(q, p1);
			if (res == 0)
				printf("NO ENCLOSING POLYGON\n");
			else
				printf("POLYGON FOUND: %d/2 PIXELS\n", res);
			break;
		case BUILD_QUADTREE:
		case ARCHIVE_QUADTREE:


			printf("THE COMMAND IS NOT IMPLEMENTED\n");
			break;
		case NONE:
			Exit = 1;
			printf("THE COMMAND IS NOT IMPLEMENTED\n");
			break;
		default:

			break;
		}
	}
}