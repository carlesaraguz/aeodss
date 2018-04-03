function checkFile(satfile)
    %% Read data:
    satdata = csvread(satfile, 2, 0);
    sattime = satdata(:, 2);
    satx = satdata(:, 3);
    saty = satdata(:, 4);
    satz = satdata(:, 5);

    %% Plot data in four plots:
    subplot(3, 3, 1);
    plot(sattime, satx);
    subplot(3, 3, 4);
    plot(sattime, saty);
    subplot(3, 3, 7);
    plot(sattime, satz);
    subplot(3, 3, [2 3 5 6 8 9]);
    [earthx, earthy, earthz] = sphere(20);
    s = surf(6371e3 * earthx, 6371e3 * earthy, 6371e3 * earthz, 'FaceColor', 'b', 'EdgeColor', 'none', 'FaceAlpha', '1');
    camlight;
    axis equal;
    hold on;
    plot3(satx, saty, satz, 'r.');


    t = linspace(0, pi, 1440);
    circle = 6571e3 * [sin(t); cos(t)];
    circle(3, :) = 0;
    rotation = (pi / 4) * ones(1, 3);
    Rx = [1, 0, 0; 0, cos(rotation(1)), -sin(rotation(1)); 0, sin(rotation(1)), cos(rotation(1))];
    Ry = [cos(rotation(2)), 0, sin(rotation(2)); 0, 1, 0; -sin(rotation(2)), 0, cos(rotation(2))];
    Rz = [cos(rotation(3)), -sin(rotation(3)), 0; sin(rotation(3)), cos(rotation(3)), 0; 0, 0, 1];
    circle = Rz * Ry * Rx * circle;
    plot3(circle(1, :), circle(2, :), circle(3, :), 'g-');

    hold off;
end
