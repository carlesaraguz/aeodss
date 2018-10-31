#include <iostream>
#include <cmath>

 #define N 3
 #define N_POINTS 20

std::vector<sf::Vector2f> getFootprint(void) const
{
    float R = 6371000.f;
    sf::Vector3f p;
    float aperture;

    float h = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);

    float lambda = Config::pi - std::asin((h / R) * std::sin(aperture / 2));
    float alpha  = Config::pi - lambda - aperture / 2;
    float r = R * std::sin(alpha) / std::sin(aperture / 2);

    float footprint_radius = R * std::sin(alpha);
    /* Also: footprint_radius = r * std::sin(aperture / 2); */

    float c_len = R * cos(alpha);
    /* Computation of cross product */
    sf::Vector3f p_n = p / h;
    std::vector<float> v = {p_n.x, p_n.y, p_n.z};
    sf::Vector3f c = v * c_len;
    std::vector<float> a = {0, v[3], -v[2]};
    std::vector<float> b (3);

    b[0] = v[1] * a[2] - v[2] * a[1];
    b[1] = v[0] * a[2] - v[2] * a[0];
    b[2] = v[0] * a[1] - v[1] * a[0];

    float step = 2 * Config::pi / N_POINTS;

    std::vector<sf::Vector3f> positions;

    for(float th = 0; th < N_POINTS; th += step) {
        float footprint_x = c.x + footprint_radius * std::cos(th) * a[1] + footprint_radius * std::sin(th) * b[1];
        float footprint_y = c.z + footprint_radius * std::cos(th) * a[2] + footprint_radius * std::sin(th) * b[2];
        float footprint_z = c.z + footprint_radius * std::cos(th) * a[3] + footprint_radius * std::sin(th) * b[3];

        sf::Vector3f fp_pos = sf::Vector3f(footprint_x, footprint_y, footprint_z);
        positions.push_back(fp_pos);
    }

    /*Here, all XYZ points of the circumference will be stored in a vector */

    return positions;
}
