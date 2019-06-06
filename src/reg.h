#include <string>
using namespace std;

class Reg {
    public:
    void leerArchivo(string archivo,string nombreSeg);
    void interactivo(string nombreSeg);
    Reg();
    ~Reg();

    private:
    int id;
};