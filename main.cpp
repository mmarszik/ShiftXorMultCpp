#include <iostream>
#include <random>
#include <vector>
#include <ctime>


using TRnd = std::mt19937_64;
using utyp   = unsigned int;
using ultyp  = unsigned long long;
using ftyp   = double;

constexpr utyp OPERATIONS = 2;
constexpr utyp SPECIMENS  = 100;
constexpr utyp TESTS      = 1000;


struct Test {
    std::vector<ultyp> values;
    utyp               buckets;
};

struct ShiftXorMult {
    utyp shiftL;
    utyp shiftR;
    ultyp mult;
    ultyp add;
};

struct Solve {
    ShiftXorMult solve[OPERATIONS];
};

struct Specimen {
    Solve   curr;
    Solve   best;
    ftyp    eval;
};

static utyp hash( ultyp v, const Solve &s, const utyp size ) {
    for( utyp i=0 ; i<OPERATIONS ; i++ ) {
        v = ( ( ( v >> s.solve[i].shiftR ) ^ ( v < s.solve[i].shiftL ) ^ v ) + s.solve[i].add ) * s.solve[i].mult;
    }
    return (utyp)( v % size );
}

static ftyp eval( const Test &t, const Solve &s ) {
    std::vector<utyp> b(t.buckets,0);
    for( utyp i=0 ; i<t.values.size() ; i++ ) {
        b[ hash( t.values[i], s , t.buckets ) ] ++;
    }
    ftyp avg = t.values.size() / t.buckets;
    ftyp e = 0;
    for( utyp i=0 ; i<b.size() ; i++ ) {
        ftyp tmp = b[i] - avg;
        e += tmp * tmp;
    }
    e = sqrt( e / b.size() );
    return e;
}

static ftyp eval( const std::vector<Test> &tests , const Solve &s ) {
    ftyp e = 0;
    for( utyp i=0 ; i<tests.size() ; i++ ) {
        e += eval( tests[i] , s );
    }
    return e / tests.size();
}

static void init( ShiftXorMult &sxm , TRnd &r ) {
    sxm.shiftL = r() % 63 - 1;
    sxm.shiftR = r() % 63 - 1;
    sxm.add    = r();
    sxm.mult   = r();
}

static void chaos( Solve &s , TRnd &r ) {
    const utyp n = r() % OPERATIONS;
    switch( r() % 10 ) {
        case 0: s.solve[n].shiftL = r() % 63 - 1;  break;
        case 1: s.solve[n].shiftR = r() % 63 - 1;  break;
        case 2: s.solve[n].add    = r();           break;
        case 3: s.solve[n].mult   = r();           break;
        case 4: s.solve[n].add    += r() % 100;    break;
        case 5: s.solve[n].mult   += r() % 100;    break;
        case 6: s.solve[n].add    -= r() % 100;    break;
        case 7: s.solve[n].mult   -= r() % 100;    break;
        case 8: s.solve[n].add    ^= 1ull<<(r()%64); break;
        case 9: s.solve[n].mult   ^= 1ull<<(r()%64); break;
    }
}

static void init( Solve &s , TRnd &r ) {
    for( utyp i=0 ; i<OPERATIONS ; i++ ) {
        init( s.solve[i] , r );
    }
}

static void init( Specimen &s , TRnd &r ,  const std::vector<Test> &tests ) {
    init( s.curr , r );
    s.best = s.curr;
    s.eval = eval( tests , s.curr );
}

static Test createTest( TRnd &r) {
    const utyp SKIP = 1000;
    Test t;
    t.buckets = r() % 20 + 10;
    utyp size = t.buckets * ( r() % 5 + 2 );
    t.values.resize(size);
    for( utyp i=0 ; i<size ; i++ ) {
        for( utyp i=0 ; i<SKIP ; i++ ) {
            r();
        }
        t.values[i] = r();
    }
    return t;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    TRnd rnd(12345);
    Specimen specs[SPECIMENS];
    std::vector<Test> tests;
    for( utyp i=0 ; i<1 ; i++ ) {
        tests.push_back( createTest(rnd) );
    }
    utyp best = 0;

    for( utyp i=0 ; i<SPECIMENS ; i++ ) {
        init( specs[i] , rnd , tests );
    }

    time_t lastShow = time(NULL);
    bool show = false;
    for( ultyp loop=0 ; true ; loop++ ) {

        if( tests.size() < TESTS && (loop+1) % (1<<18) == 0 ) {
            tests.push_back( createTest(rnd) );
            best = 0;
            for( utyp i=0 ; i<SPECIMENS ; i++ ) {
                specs[i].curr = specs[i].best;
                specs[i].eval = eval( tests , specs[i].curr );
                if( specs[i].eval < specs[best].eval ) {
                    best = i;
                }
            }
            show = true;
        }

        for( utyp i=0 ; i<SPECIMENS ; i++ ) {
            chaos( specs[i].curr , rnd );
            ftyp tmp = eval( tests , specs[i].curr );
            if( tmp <= specs[i].eval ) {
                specs[i].best = specs[i].curr;
                specs[i].eval = tmp;
                if( specs[i].eval < specs[best].eval ) {
                    best = i;
                    show = true;
                }
            } else if( rnd() & 2 ) {
                specs[i].curr = specs[i].best;
            }
        }

        if( show || ( (loop & ((1u<<10)-1)) == 0 && time(NULL) - lastShow >= 60 ) ) {
            std::cout << "loops: " << loop             << std::endl;
            std::cout << "tests: " << tests.size()     << std::endl;
            std::cout << "eval: "  << specs[best].eval << std::endl;
            for( utyp i=0 ; i<OPERATIONS ; i++ ) {
                std::cout << "shiftL: " << specs[best].best.solve[i].shiftL << std::endl;
                std::cout << "shiftR: " << specs[best].best.solve[i].shiftR << std::endl;
                std::cout << "add:    " << specs[best].best.solve[i].add    << std::endl;
                std::cout << "mult:   " << specs[best].best.solve[i].mult   << std::endl;
            }
            show = false;
            lastShow = time(NULL);
        }


    }


    return 0;
}

