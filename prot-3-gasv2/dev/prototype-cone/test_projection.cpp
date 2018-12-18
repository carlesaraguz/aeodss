#include <iostream>
#include <cmath>

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
    sf::Vector3f v = sf::Vector3f(p_n.x, p_n.y, p_n.z);
    sf::Vector3f c = v * c_len;

    /* When v = [1 0 0], a will be a = [0 0 0] using our perpendicular vector generator method.
     * For this reason, we check that v = [1 0 0] and in this case we force a = [0 1 0] that is
     * always perpendicular to v. And then, we generate b with the cross product.
     **/

    if(v == sf::Vector3f(1, 0, 0)) {
        sf::Vector3f a = sf::Vector3f(0, 1, 0);
    } else {
        sf::Vector3f a = sf::Vector3f(0, v.z, -v.y);
    }

    a = a / std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    sf::Vector3f b = sf::Vector3f();

    b.x = v.y * a.z - v.z * a.y;
    b.y = v.x * a.z - v.z * a.x;
    b.z = v.x * a.y - v.y * a.x;

    b = b / std::sqrt(b.x * b.x + b.y * b.y + b.z * b.z);


    float step = 2 * Config::pi / N_POINTS;

    std::vector<sf::Vector3f> positions;

    sf::Vector3f n_pole = sf::Vector3f(0, 0, R);
    sf::Vector3f s_pole = sf::Vector3f(0, 0, -R);


    bool north_included = false;
    bool south_included = false;

    float distance_to_north = std::sqrt((c.x - n_pole.x) * (c.x - n_pole.x) + (c.y - n_pole.y) * (c.y - n_pole.y)
        + (c.z - n_pole.z) * (c.z - n_pole.z));
    float distance_to_south = std::sqrt((c.x - s_pole.x) * (c.x - s_pole.x) + (c.y - s_pole.y) * (c.y - s_pole.y)
            + (c.z - s_pole.z) * (c.z - s_pole.z));

    if(distance_to_north < footprint_radius) {
        north_included = true;
    } else if(distance_to_south < footprint_radius) {
        south_included = true;
    }

    for(float th = 0; th < N_POINTS; th += step) {
        float footprint_x = c.x + footprint_radius * std::cos(th) * a.x + footprint_radius * std::sin(th) * b.x;
        float footprint_y = c.z + footprint_radius * std::cos(th) * a.y + footprint_radius * std::sin(th) * b.y;
        float footprint_z = c.z + footprint_radius * std::cos(th) * a.z + footprint_radius * std::sin(th) * b.z;

        sf::Vector3f fp_pos = sf::Vector3f(footprint_x, footprint_y, footprint_z);
        sf::Vector2f proj   = AgentMotion::getProjection2D(fp_pos, VirtualTime::now());

        sf::Vector2f prev = positions.back();

        if(north_included) {

            float border_y = std::abs(prev.y - proj.y) / 2;

            if(prev.x > proj.x) {
                positions.push_back(sf::Vector2f(Config::world_width, border_y));
                positions.push_back(sf::Vector2f(Config::world_width, Config::world_height));
                positions.push_back(sf::Vector2f(0, Config::world_height));
                positions.push_back(sf::Vector2f(0, border_y));
            } else {
                positions.push_back(sf::Vector2f(0, border_y));
                positions.push_back(sf::Vector2f(0, Config::world_height));
                positions.push_back(sf::Vector2f(Config::world_width, Config::world_height));
                positions.push_back(sf::Vector2f(Config::world_width, border_y));
            }
        }

        if(south_included) {

            float border_y = std::abs(prev.y - proj.y) / 2;

            if(prev.x > proj.x) {
                positions.push_back(sf::Vector2f(Config::world_width, border_y));
                positions.push_back(sf::Vector2f(Config::world_width, 0));
                positions.push_back(sf::Vector2f(0, 0));
                positions.push_back(sf::Vector2f(0, border_y));
            } else {
                positions.push_back(sf::Vector2f(0, border_y));
                positions.push_back(sf::Vector2f(0, 0));
                positions.push_back(sf::Vector2f(Config::world_width, 0));
                positions.push_back(sf::Vector2f(Config::world_width, border_y));
            }
        }

        bool is_splitted = false;

        float x_distance = std::abs(proj.x - prev.x);

        if(x_distance > Config::world_width / 2) {
            is_splitted = true;
        }

        if(is_splitted){
            /* Compute border point */
            float border_y = std::abs(prev.y - proj.y) / 2;

            if(prev.x > proj.x) {
                positions.push_back(sf::Vector2f(Config::world_width, border_y));
                positions.push_back(sf::Vector2f(Config::world_width, 0));
                positions.push_back(sf::Vector2f(0, 0));
                positions.push_back(sf::Vector2f(0, border_y));
            } else {
                positions.push_back(sf::Vector2f(0, border_y));
                positions.push_back(sf::Vector2f(0, world_height));
                positions.push_back(sf::Vector2f(Config::world_width, world_height));
                positions.push_back(sf::Vector2f(Config::world_width, border_y));
            }
        }
        if(proj.y != Config::world_height || proj.y != 0){
            positions.push_back(proj);
        }
    }

    return positions;
}
