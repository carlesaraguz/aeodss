function checkFile(satfile)
    satdata = csvread(satfile, 2, 0);
    sattime = satdata(:, 2);
    satx = satdata(:, 3);
    saty = satdata(:, 4);
    satz = satdata(:, 5);
    subplot(3, 3, 1);
    plot(sattime, satx);
    subplot(3, 3, 4);
    plot(sattime, saty);
    subplot(3, 3, 7);
    plot(sattime, satz);
    subplot(3, 3, [2 3 5 6 8 9]);
    % plot3(satx, saty, satz);
    earth = 6371e3 * sphere(40);
    surf(earth);
end
