'use strict';

const byId = id => document.getElementById(id);
const lines = [[0,1,2],[3,4,5],[6,7,8],[0,3,6],[1,4,7],[2,5,8],[0,4,8],[2,4,6]];
const names = { minimax: 'Минимакс', alphabeta: 'Альфа-бета + кеш' };
const errors = {
    wrong_turn: 'Для хода O крестиков должно быть ровно на один больше, чем ноликов.',
    finished: 'Эта партия уже завершена.',
    invalid_input: 'Введите девять символов X, O или точек.',
};
const number = new Intl.NumberFormat('ru', { maximumFractionDigits: 2 });
const coordinates = index => [Math.floor(index / 3) + 1, index % 3 + 1];

let board = Array(9).fill('.');
let mode = 'play';
let hint = null;
let report = null;
let busy = false;
let revision = 0;

const cells = board.map((_, index) => {
    const cell = document.createElement('button');
    cell.type = 'button';
    cell.className = 'cell';
    const mark = document.createElement('span');
    mark.className = 'mark';
    mark.setAttribute('aria-hidden', 'true');
    cell.append(mark);
    cell.addEventListener('click', () => clickCell(index));
    byId('board').append(cell);
    return cell;
});

function winningLine() {
    return lines.find(([a, b, c]) => board[a] !== '.' && board[a] === board[b] && board[a] === board[c]);
}

function currentTurn() {
    return board.filter(cell => cell === 'X').length === board.filter(cell => cell === 'O').length ? 'X' : 'O';
}

function render() {
    const playing = mode === 'play';
    const win = winningLine();
    const done = win || !board.includes('.');
    const turn = currentTurn();
    const canSearch = !busy && !done && turn === 'O';
    byId('status').textContent = busy ? 'Поиск хода…' : win ? 'Победа ' + board[win[0]] : done ? 'Ничья'
        : playing ? (turn === 'X' ? 'Ваш ход' : 'Ход компьютера') : 'Редактор поля';
    byId('turn').textContent = playing && !done ? turn : '';
    byId('editor').hidden = playing;
    byId('play').setAttribute('aria-pressed', String(playing));
    byId('analysis').setAttribute('aria-pressed', String(!playing));
    byId('x-label').textContent = playing ? 'Вы' : 'Крестик';
    byId('o-label').textContent = playing ? 'Компьютер' : 'Нолик';
    byId('reset').lastElementChild.textContent = playing ? 'Новая игра' : 'Очистить';
    byId('recommend').hidden = playing && (turn !== 'O' || !!done);
    byId('recommend').disabled = busy;
    byId('compare').disabled = playing ? !canSearch : busy;
    byId('method').disabled = busy;
    cells.forEach((cell, index) => {
        const [row, column] = coordinates(index);
        const mark = board[index];
        cell.dataset.mark = mark;
        cell.classList.toggle('win', win?.includes(index) ?? false);
        cell.classList.toggle('hint', hint === index && mark === '.');
        cell.setAttribute('aria-disabled', String(playing && (busy || !!done || mark !== '.' || turn !== 'X')));
        const label = mark !== '.' ? mark : hint === index ? 'рекомендованный ход O' : 'пусто';
        cell.setAttribute('aria-label', `Строка ${row}, столбец ${column}, ${label}`);
    });
}

function clearReport() {
    ++revision;
    busy = false;
    hint = null;
    report = null;
    byId('message').textContent = '';
    byId('results-info').textContent = 'Здесь появятся координаты хода и время поиска.';
    byId('metrics').replaceChildren();
    byId('csv').disabled = true;
}

function setPosition(position) {
    board = [...position];
    clearReport();
    byId('position').value = position;
    render();
}

function clickCell(index) {
    if (cells[index].getAttribute('aria-disabled') === 'true') return;
    if (mode === 'analysis') {
        const next = { '.': 'X', X: 'O', O: '.' };
        board[index] = next[board[index]];
        setPosition(board.join(''));
        return;
    }
    board[index] = 'X';
    clearReport();
    render();
    if (!winningLine() && board.includes('.')) run(byId('method').value, true);
}

function showReport(position, results) {
    report = { position, results };
    byId('results-info').textContent = `${position} · ${results[0].repetitions} запуск(ов) на алгоритм`;
    byId('metrics').replaceChildren();
    for (const result of results) {
        const [row, column] = coordinates(result.move);
        const metric = document.createElement('div');
        metric.className = 'metric';
        const heading = document.createElement('div');
        heading.className = 'metric-head';
        const name = document.createElement('strong');
        name.textContent = names[result.method];
        const time = document.createElement('span');
        time.className = 'metric-time';
        time.textContent = number.format(result.microseconds) + ' мкс';
        const details = document.createElement('p');
        details.textContent = `Ход (${row}, ${column}) · оценка ${result.score} · узлы ${result.nodes} · отсечения ${result.cuts} · из кеша ${result.hits}`;
        heading.append(name, time);
        metric.append(heading, details);
        byId('metrics').append(metric);
    }
    byId('csv').disabled = false;
}

async function run(method, applyMove = false) {
    if (busy) return;
    clearReport();
    const ticket = revision;
    busy = true;
    const position = board.join('');
    render();
    try {
        const params = new URLSearchParams({ method, board: position });
        const response = await fetch('/api/solve?' + params);
        const result = await response.json();
        if (ticket !== revision) return;
        if (!response.ok) throw new Error(errors[result.error] || 'Не удалось выполнить поиск.');
        showReport(position, result);
        if (applyMove) board[result[0].move] = 'O';
        else hint = result[0].move;
    } catch (error) {
        if (ticket === revision) byId('message').textContent = error instanceof TypeError ? 'Нет связи с сервером.' : error.message;
    } finally {
        if (ticket === revision) {
            busy = false;
            render();
        }
    }
}

byId('play').onclick = () => {
    if (mode === 'play') return;
    mode = 'play';
    setPosition('.........');
};
byId('analysis').onclick = () => {
    if (mode === 'analysis') return;
    mode = 'analysis';
    setPosition(board.join(''));
};
byId('reset').onclick = () => setPosition('.........');
byId('example').onclick = () => setPosition('XX..O....');
byId('load').onclick = () => {
    const position = byId('position').value.trim().toUpperCase();
    if (!/^[XO.]{9}$/.test(position)) {
        byId('message').textContent = errors.invalid_input;
        return;
    }
    setPosition(position);
};
byId('position').addEventListener('keydown', event => {
    if (event.key === 'Enter') byId('load').click();
});
byId('recommend').onclick = () => run(byId('method').value, mode === 'play');
byId('compare').onclick = () => run('compare');
byId('csv').onclick = () => {
    if (!report) return;
    const header = 'position,method,row,column,score,nodes,cuts,cache_hits,repetitions,mean_microseconds';
    const rows = report.results.map(result => [
        report.position, result.method, ...coordinates(result.move), result.score,
        result.nodes, result.cuts, result.hits, result.repetitions, result.microseconds,
    ].join(','));
    const url = URL.createObjectURL(new Blob([[header, ...rows].join('\r\n') + '\r\n'], { type: 'text/csv' }));
    const link = document.createElement('a');
    link.href = url;
    link.download = 'laba2-benchmark.csv';
    link.click();
    setTimeout(() => URL.revokeObjectURL(url), 1000);
};
render();
