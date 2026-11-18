'use strict';

const http = require('node:http');
const fs = require('node:fs');
const path = require('node:path');
const { execFile } = require('node:child_process');

const executableName = process.platform === 'win32' ? 'solver.exe' : 'solver';
const solver = [path.join('build', 'Release', executableName), path.join('build', executableName)]
    .map(file => path.join(__dirname, file)).find(file => fs.existsSync(file));

if (!solver) {
    console.error('Сначала соберите C++: cmake -S . -B build && cmake --build build --config Release');
    process.exit(1);
}

const assets = new Map([
    ['/', ['index.html', 'text/html']],
    ['/style.css', ['style.css', 'text/css']],
    ['/app.js', ['app.js', 'text/javascript']],
]);

function reply(response, status, data) {
    response.writeHead(status, { 'Content-Type': 'application/json; charset=utf-8' });
    response.end(JSON.stringify(data));
}

http.createServer((request, response) => {
    const url = new URL(request.url, 'http://localhost');
    if (request.method !== 'GET') return reply(response, 405, { error: 'method' });

    const asset = assets.get(url.pathname);
    if (asset) {
        const [file, type] = asset;
        response.writeHead(200, { 'Content-Type': type + '; charset=utf-8' });
        fs.createReadStream(path.join(__dirname, 'web', file)).pipe(response);
        return;
    }
    if (url.pathname !== '/api/solve') return reply(response, 404, { error: 'not_found' });

    const method = url.searchParams.get('method');
    const board = url.searchParams.get('board') || '';
    if (!['minimax', 'alphabeta', 'compare'].includes(method) || !/^[XO.]{9}$/.test(board)) {
        return reply(response, 400, { error: 'invalid_input' });
    }
    execFile(solver, [method, board], { timeout: 10000, windowsHide: true }, (error, stdout) => {
        try {
            const result = JSON.parse(stdout);
            if (!error) return reply(response, 200, result);
            if (error.code === 2 && result.error) return reply(response, 400, result);
        } catch {
            // The solver can fail before writing a JSON result.
        }
        reply(response, 500, { error: 'solver_failed' });
    });
}).listen(Number(process.env.PORT || 3000), '127.0.0.1', () => {
    console.log('http://127.0.0.1:' + (process.env.PORT || 3000));
});
