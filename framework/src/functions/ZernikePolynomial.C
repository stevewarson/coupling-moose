#include "ZernikePolynomial.h"
#include "ExtraFunctions.h"
#include "math.h"

template<>
InputParameters validParams<ZernikePolynomial>()
{
  InputParameters params = validParams<Function>();
  /* The Zernike polynomials are orthogonal on the unit circle, but because
     the domain may not exist on a circle with radius unity, we need to
     normalize the r-coordinate passed into this function to unity. */
  params.addRequiredParam<Real>("radius",
    "Radius needed for Zernike polynomial normalization");

  /* To find theta, we must know where the center of the domain's circle is
     relative to the coordinate. So, you need to pass in the center of the
     circle in the physical domain. */
  params.addRequiredParam<std::vector<Real>>("center",
    "center coordinates (x, y) of circle");
  params.addParam<bool>("dbg", false, "Print debug output");
  return params;
}

ZernikePolynomial::ZernikePolynomial(const InputParameters & parameters) :
    Function(parameters),
    _radius(parameters.get<Real>("radius")),
    _center(parameters.get<std::vector<Real>>("center")),
    _dbg(parameters.get<bool>("dbg"))
{
}

ZernikePolynomial::~ZernikePolynomial()
{
}

Real
ZernikePolynomial::value(Real t, const Point & p)
{
  /* Because we need to pass in the orders of the Zernike polynomial,
     the parameter list for this method does not suffice. */
  mooseWarning("value() in ZernikePolynomial should not be used");
  return 0.0;
}

Real
ZernikePolynomial::getPolynomialValue(Real t, Real x, Real y, int m, int n)
{

  // Check that the (x, y) point provided is not outside the circle.
  Real distance = (x - _center[0]) * (x - _center[0]) +
    (y - _center[1]) * (y - _center[1]);

//  if (distance > _radius) mooseWarning("Point provided to ZernikePolynomial "
//    "is outside of the specified circle.");

  // Check that m is one of -n, -n + 2, -n + 4, ..., n
  Real is_int = (n - abs(m)) / 2;
  if (is_int - floor(is_int) > 0.001)
    mooseWarning("m order is not valid; m = -n, -n + 2, -n + 4, ..., n in "
    " Zernike order");

  // Check that n is non-negative
  if (n < 0)
    mooseWarning("n must be non-negative in Zernike order");

  /* Normalize the coordinates by first relocating the circle to (0, 0)
     and then scaling the coordinates individually so that the radius is unity
     by dividing by the radius. */
  Real x_norm = (x - _center[0]) / _radius;         // Normalized position
  Real y_norm = (y - _center[1]) / _radius;         // Normalized position

  Real theta = atan2(y_norm, x_norm);
  Real r = sqrt(x_norm * x_norm + y_norm * y_norm);

  Real N = 0.0; // normalization factor multiplied by cosine or sine
  Real f1, f2;

  if (m == 0)
    N = sqrt(n + 1) * cos(m * theta);
  else if (m < 0)
    N = - sqrt(2.0 * (n + 1)) * sin(m * theta);
  else
    N = sqrt(2.0 * (n + 1)) * cos(m * theta);

  Real Rnm = 0.0;
  for (int s = 0; s <= (n - abs(m)) / 2; ++s)
  {
    f1 = factorial(0.5 * (n + abs(m)) - s);
    f2 = factorial(0.5 * (n - abs(m)) - s);
    Rnm += pow(-1.0, s) * (factorial(n - s) / (factorial(s) * f1 * f2)) * pow(r, n - 2.0 * s);

  }

  if (_dbg)
  {
    _console << "Normalization factor: " << N << std::endl;
    _console << "theta: " << theta << std::endl;
    _console << "Normalized coordinates (x, y): (" <<
      x_norm << ", " << y_norm << ")" << std::endl;
  }

  return N * Rnm / sqrt(M_PI);
}
