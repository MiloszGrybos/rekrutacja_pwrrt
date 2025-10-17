#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>


using namespace std;

struct Point{
    double x,y;
};

vector<Point> readPoints(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Nie można otworzyć pliku: " + filename);
    }
    
    int n;
    file >> n;
    
    vector<Point> points;
    points.reserve(n);
    
    for (int i = 0; i < n; ++i) {
        double x, y;
        file >> x >> y;
        points.push_back({x, y});
    }
    
    return points;
}

void printPoints(const vector<Point>& points) {
    for (Point p : points) {
        cout << "(" << p.x << ", " << p.y << ")" << endl;
    }
}

// Funkcja wyliczająca, czy kąt między P2, a P3, jest większy (return 1), od kąta między P1, a P2 (return -1)
// wykorzystuję tutaj tangensy obu par, po przekształceniu algebraicznym wychodzi, działanie zmiennej tangens_comp
int angle(Point p1, Point p2, Point p3){
    double tangens_comp = (p3.y - p2.y)*(p2.x - p1.x) - (p2.y - p1.y)*(p3.x - p2.x);
    if (tangens_comp > 0){
        return 1;
    }
    else if (tangens_comp < 0){
        return -1;
    }
    else{
        return 0;
    }
}

// Szukamy najniżej położonego punktu
int findLowest(vector<Point>& points) {
    int minIndex = 0;
    for (int i = 1; i < points.size(); i++) {
        if (points[i].y < points[minIndex].y || (points[i].y == points[minIndex].y && points[i].x < points[minIndex].x)) {
            minIndex = i;
        }
    }
    return minIndex;
}


// Używamy algorytmu Grahama
vector<Point> graham(vector<Point>& points){
    //Wybieramy sobie nasz najnizszy punkt
    swap(points[0], points[findLowest(points)]);
    Point p0 = points[0];

    vector<Point> cover;

    //Sortujemy pozostałe punkty według tego, pod jakim leżą kątem względem najniższego puntku
    //jeżeli są pod takim samym kątem to bierzemy ten bliższy
    sort(points.begin() + 1 ,points.end(), [&p0](const Point& p1, const Point& p2){
        int result = angle(p0, p1, p2);
        if (result == 0){
            double distance1 = (p1.x - p0.x)*(p1.x - p0.x) + (p1.y - p0.y)*(p1.y - p0.y);
            double distance2 = (p2.x - p0.x)*(p2.x - p0.x) + (p2.y - p0.y)*(p2.y - p0.y);

            return distance1 < distance2;
        }
        return result == 1;
    } );

    //Przechodzimy teraz po posortowanych punktach, musimy porównywać ostatni dodany punkt oraz jego poprzednika
    //jeżeli kąt okaże się mniejszy, to znaczy, że ten punkt nie należy do naszej szukanej otoczki
    for (int i=0; i < points.size(); i++){
        while (cover.size() >=2){
            Point top = cover.back();
            Point second_top = cover[cover.size() -2];

            if (angle(second_top, top, points[i]) != 1){
                cover.pop_back();
            }
            else{
                break;
            }
        }
        cover.push_back(points[i]);
    }
    return cover;
}

//Funkcja obliczająca odległość punktu, od prostej, na której znajdują się dwa znane punkty
double distancePointToLine(const Point& A, const Point& B, const Point& P) {
    double dx = B.x - A.x;
    double dy = B.y - A.y;
    
    double a = abs(dy * P.x - dx * P.y + B.x * A.y - B.y * A.x);
    double b = sqrt(dy * dy + dx * dx);
    
    return a / b;
}

//Funkcja znajdująca dwie najbliższe proste równoległe obejmujące zbiór
double minParallelLines(const vector<Point>& cover) {
    int n = cover.size();
    if (n < 3) return 0;
    
    double min = numeric_limits<double>::max();
    
    for (int i = 0; i < n; i++) {
        Point A = cover[i];

        //Ten indeks wynika z tego, że znajdując się w grupie modulo n nie przekroczymy zakresu a jednocześnie obejdziemy wszystkie opcje
        Point B = cover[(i + 1) % n];
        
        // Dla każdej krawędzi szukamy tych prostych, które spełniają warunki zadania
        //korzystamy z otoczki wypukłej policzonej wcześniej, ponieważ nie chcemy szukać wszystkich prostych,
        //tylko te, które będą sensownymi kandydatami
        double max_dist = 0;
        for (int j = 0; j < n; j++) {
            if (j == i || j == (i + 1) % n) continue;
            
            double dist = distancePointToLine(A, B, cover[j]);
            //Bierzemy pod uwagę tylko te proste, które przechodzą przez najbardziej oddalony wierzchołek
            //inaczej nie zawierają one wszystkich punktów
            if (dist > max_dist) {
                max_dist = dist;
            }
        }
        
        if (max_dist < min) {
            min = max_dist;
        }
    }
    
    return min;
}

pair<Point, Point> closestBruteForce(const vector<Point>& points){


    pair<Point, Point> closestPair = {points[0], points[1]};
    double minDist = numeric_limits<double>::max();
    
    for (size_t i = 0; i < points.size(); ++i) {
        for (size_t j = i + 1; j < points.size(); ++j) {
            double dist = (points[i].x - points[j].x) * (points[i].x - points[j].x) + (points[i].y - points[j].y) * (points[i].y - points[j].y);
            
            if (dist < minDist) {
                minDist = dist;
                closestPair = {points[i], points[j]};
            }
        }
    }
    return closestPair;
}

//Funkcja szukająca dwóch najbliższych punktów
pair<Point, Point> closestPoints(vector<Point>& points){

    //Sortowaanie po x
    sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
        if(a.x == b.x) return a.y < b.y;
            return a.x < b.x;
    });

    pair<Point,Point> closestPair;

    if(points.size() <= 3){
        return closestBruteForce(points);
    }

    //Bierzemy punkt, który po lewej i prawej stronie ma tyle samo innych punktów
    size_t mid = points.size() / 2;
    Point midPoint = points[mid];

    //Tworzymy podzbiory - lewy i prawy
    vector<Point> left(points.begin(), points.begin() + mid);
    vector<Point> right(points.begin() + mid, points.end());

    //Wykonujemy rekurencyjnie algorytm
    pair<Point,Point> leftClosest = closestPoints(left);
    pair<Point,Point> rightClosest = closestPoints(right);

    double leftDistance = (leftClosest.second.x - leftClosest.first.x)*(leftClosest.second.x - leftClosest.first.x) + (leftClosest.second.y - leftClosest.first.y)*(leftClosest.second.y - leftClosest.first.y);
    double rightDistance = (rightClosest.second.x - rightClosest.first.x)*(rightClosest.second.x - rightClosest.first.x) + (rightClosest.second.y - rightClosest.first.y)*(rightClosest.second.y - rightClosest.first.y);

    double delta;
    
    //Sprawdzamy, czy lewa strona miała bliższą parę, czy prawa
    if (leftDistance <= rightDistance) {
        closestPair = leftClosest;
        delta = leftDistance;
    } else {
        closestPair = rightClosest;
        delta = rightDistance;
    }

    //Sprawdzamy, czy istnieją punkty, które są oddalone od naszego "środkowego" punktu o mniej niż delta
    //W ten sposób tworzym "pasek", na którym będzie trzeba jeszcze sprawdzić punkty
    vector<Point> middlePart;
    for(const Point& p : points){
        if(abs(p.x - midPoint.x) < delta){
            middlePart.push_back(p);
        }
    }

    //Sortujemy wcześniej znalezione punkty wg współrzędnej y
    sort(middlePart.begin(), middlePart.end(), [](const Point& a, const Point& b){
        if(a.y == b.y){
            return a.x < b.x;
        }
        return a.y < b.y;
    });

    //Sprawdzamy czy w "pasku" są punkty o mniejszej odległości niż wcześniej znaleziona para
    //nie musimy sprawdzac więcej niż 6 punktów dla punktu
    for (size_t i = 0; i < middlePart.size(); ++i) {
        for (size_t j = i + 1; j < middlePart.size() && j < i + 7; ++j) {
            if (middlePart[j].y - middlePart[i].y >= delta) {
                break;
            }
            
            double currentDist = (middlePart[i].x - middlePart[j].x)*(middlePart[i].x - middlePart[j].x) + (middlePart[i].y - middlePart[j].y)*(middlePart[i].y - middlePart[j].y);

            if (currentDist < delta) {
                delta = currentDist;
                closestPair = {middlePart[i], middlePart[j]};
            }
        }
        
    }
    return closestPair;
}

int main() {
    try {
        vector<Point> points = readPoints("points.txt");

        cout << "\nOtoczka: " << endl;
        vector<Point> cover = graham(points);
        printPoints(cover);

        cout << "\nNajmniejsza odleglosc: " << minParallelLines(cover) << endl;

        pair<Point,Point> result = closestPoints(points);
        double distance = sqrt((result.second.x - result.first.x)*(result.second.x - result.first.x) + (result.second.y - result.first.y)*(result.second.y - result.first.y));

        cout << "\nNajbliższe punkty: (" << result.first.x << ", " << result.first.y << "), (" << result.second.x << ", " << result.second.y << "), Odleglosc: " << distance << endl;
        
    } catch (const exception& e) {
        cout << "Błąd: " << e.what() << endl;
    }
    
    return 0;
}

