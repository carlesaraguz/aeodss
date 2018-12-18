clc;
clear all

R = 6371;   % Radius of the Earth in km.

% Show plot of Earth:
[ESx, ESy, ESz] = sphere(40);   % Earth sphere
mesh(R * ESx, R * ESy, R * ESz);
hold on;

for i = [1:50]  % Creates 10 test cases.
    % ----------------------------------------------------------------------------------------------
    aperture = (pi / 180) * 80;                 % Instrument aperture (β) in radians (80 deg.)
    % aperture = (pi / 180) * unifrnd(30, 80);    % Instrument aperture (β) in radians (30 to 80 deg.)
    p = unifrnd(-1, 1, 1, 3);                   % Position of satellite agent.
    p = (R + 900) * (p / norm(p));
    % p = unifrnd(R + 500, R + 900) * (p / norm(p));

    h = norm(p);                                        % Distance from p to center of Earth.
    lambda = pi - asin((h / R) * sin(aperture / 2));    % Angle that form r and R.
    alpha = pi - lambda - (aperture / 2);               % Angle that form R and h.
    r = R * sin(alpha) / sin(aperture / 2);             % Radius of small sphere that intersetcs as a cone of aperture.

    footprint_radius = R * sin(alpha);          % Radius of the footprint circle.
    footprint_radius = r * sin(aperture / 2);   % Radius of the footprint circle.

    c_len = R * cos(alpha);     % Distance of point c from center of Earth.
    v = p / norm(p);            % Unitary vector with same direction than p.
    c = v * c_len;              % Point C.
    a = [v(3) v(3) (-v(1) - v(2))];
    if a == [0 0 0]
        a = [(-v(2)-v(3)) v(1) v(1)];
    endif
    b = cross(a, v);            % Vector B --> B ⟂ v, B ⟂ A.

    theta = [0:10:360] * pi / 180;
    footprint_x = c(1) + footprint_radius * cos(theta) * a(1) + footprint_radius * sin(theta) * b(1);
    footprint_y = c(2) + footprint_radius * cos(theta) * a(2) + footprint_radius * sin(theta) * b(2);
    footprint_z = c(3) + footprint_radius * cos(theta) * a(3) + footprint_radius * sin(theta) * b(3);
    fp = [footprint_x, footprint_y, footprint_z];  % The footprint circle.

    plot3(footprint_x, footprint_y, footprint_z, "b-", "linewidth", 3);
    plot3(p(1), p(2), p(3), "b*")
    plot3([c(1) p(1)], [c(2) p(2)], [c(3) p(3)], "r")
    axis square;
endfor

hold off;
