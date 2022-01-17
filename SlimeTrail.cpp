#include "SlimeTrail.h"
#include "ui_SlimeTrail.h"

#include <QDebug>
#include <QMessageBox>
#include <QActionGroup>
#include <QSignalMapper>

SlimeTrail::Player otherPlayer(SlimeTrail::Player player) {
    return (player == SlimeTrail::RedPlayer ?
                SlimeTrail::BluePlayer : SlimeTrail::RedPlayer);
}

SlimeTrail::SlimeTrail(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::SlimeTrail),
      m_player(SlimeTrail::RedPlayer) {

    ui->setupUi(this);

    QObject::connect(ui->actionNew, SIGNAL(triggered(bool)), this, SLOT(reset()));
    QObject::connect(ui->actionQuit, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
    QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(showAbout()));

    QSignalMapper* map = new QSignalMapper(this);
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QString holeName = QString("hole%1%2").arg(row).arg(col);
            Hole* hole = this->findChild<Hole*>(holeName);
            Q_ASSERT(hole != nullptr);
            Q_ASSERT(hole->row() == row && hole->col() == col);

            m_board[row][col] = hole;

            int id = row * 8 + col;
            map->setMapping(hole, id);
            QObject::connect(hole, SIGNAL(clicked(bool)), map, SLOT(map()));
        }
    }
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QObject::connect(map, SIGNAL(mapped(int)), this, SLOT(play(int)));
#else
    QObject::connect(map, SIGNAL(mappedInt(int)), this, SLOT(play(int)));
#endif

    // When the turn ends, switch the player.
    QObject::connect(this, SIGNAL(turnEnded()), this, SLOT(switchPlayer()));

    this->reset();

    this->adjustSize();
    this->setFixedSize(this->size());
}

SlimeTrail::~SlimeTrail() {
    delete ui;
}

void SlimeTrail::play(int id) {
    Hole* hole = m_board[id / 8][id % 8];
    qDebug() << "clicked on: " << hole->objectName();


    if(hole->state() == hole->EmptyState && hole->isDisponible()){
        hole->setState(Hole::WhiteState);

        bool avaibleSpace = hole->verifyHolesDisponible(hole->col(), hole->row(), m_board);

        if(m_board[7][0]->state() == hole->WhiteState){
            QMessageBox::information(this, tr("Parabéns"), tr("O jogador VERMELHO venceu!!!"));
            this->reset();
        }else if(m_board[0][7]->state() == hole->WhiteState){
            QMessageBox::information(this, tr("Parabéns"), tr("O jogador AZUL venceu!!!"));
            this->reset();
        }
        else if(avaibleSpace){
            hole->setDisponible(false);
            emit turnEnded();
        }else{
            QMessageBox::information(this, tr("Empate"), tr("Nenhum dos dois jogando venceu!!!"));
            this->reset();
        }
    }
}

void Hole::colorPreviousHouse(Hole* board[8][8], int rol, int col){

    if(board[rol][col]->state() == WhiteState){
        board[rol][col]->setState(BlackState);
        board[rol][col]->setDisponible(false);
    }
}

bool Hole::verifyHolesDisponible(int col, int rol, Hole* board[8][8]){    
    int casas = 0; //Se entrar em apenas um desses ifs, significa que achou somente a casa branca, portanto nao a casas disponiveis;
    clearHolesDisponible(board);

    //Coluna
    int next_previus = col + 1;
    if((next_previus <= 7 && (( board[rol][next_previus]->state() != BlackState)))){
        board[rol][next_previus]->setDisponible(true);
        colorPreviousHouse(board, rol, next_previus);
        casas++;
    }


    next_previus = col - 1;
    if((next_previus >= 0 &&( board[rol][next_previus]->state() != BlackState))){
        board[rol][next_previus]->setDisponible(true);
        colorPreviousHouse(board, rol, next_previus);
        casas++;
    }


    //Linha
    next_previus = rol + 1;
    if((next_previus <= 7 && ( board[next_previus][col]->state() != BlackState))){
        board[next_previus][col]->setDisponible(true);
        colorPreviousHouse(board, next_previus, col );
        casas++;
    }


    next_previus = rol - 1;
    if((next_previus >= 0 && board[next_previus][col]->state() != BlackState)){
        board[next_previus][col]->setDisponible(true);
        colorPreviousHouse(board, next_previus, col );
        casas++;
    }


    //diagonais
    //Inferiores
    if((col+1) <= 7 && (rol+1) <= 7 &&(board[(rol+1)][col+1]->state() != BlackState) ){
        board[(rol+1)][col+1]->setDisponible(true);
        colorPreviousHouse(board, (rol+1), (col+1) );
        casas++;
    }


    if((col-1) >= 0 && (rol+1) <= 7 &&( board[(rol+1)][col-1]->state() != BlackState)){
        board[(rol+1)][col-1]->setDisponible(true);
        colorPreviousHouse(board, (rol+1), (col-1));
        casas++;
    }


    //Superiores
    if((col-1) >= 0 && (rol-1) >= 0 &&(board[(rol-1)][col-1]->state() != BlackState)){
        board[(rol-1)][col-1]->setDisponible(true);
        colorPreviousHouse(board, (rol-1), (col-1));
        casas++;
    }


    if(((col+1) <= 7) && ((rol-1) >= 0) && (board[(rol-1)][col+1]->state() != BlackState)){
        board[(rol-1)][col+1]->setDisponible(true);
        colorPreviousHouse(board, (rol-1), (col+1));
        casas++;
    }


    return (casas > 1);

}

void Hole::clearHolesDisponible(Hole* board[8][8]){
    for(int i = 0; i < 8; i++){
        for(int j = 0; j <8; j++){
            board[i][j]->setDisponible(false);
        }
    }
}

void SlimeTrail::switchPlayer() {
    // Switch the player.
    m_player = otherPlayer(m_player);

    // Finally, update the status bar.
    this->updateStatusBar();
}

void SlimeTrail::reset() {
    // Reset board.

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            Hole* hole = m_board[row][col];
            hole->reset();
        }
    }

    Hole* hole = m_board[3][4];
    hole->verifyHolesDisponible(4, 3, m_board);

    // Mark the starting position.
    m_board[3][4]->setState(Hole::WhiteState);

    // Reset the player.
    m_player = SlimeTrail::RedPlayer;

    // Finally, update the status bar.
    this->updateStatusBar();
}

void SlimeTrail::showAbout() {
    QMessageBox::information(this, tr("Sobre"), tr("Rastros\n\nDesenvolvido por:\nPablo Vasconcelos da Cruz: https://github.com/Pablo321123\n Pedro Costa Calazans: https://github.com/pedrocostacalazans"));
}

void SlimeTrail::updateStatusBar() {
    QString player(m_player == SlimeTrail::RedPlayer ? "Vermelho" : "Azul");
    ui->statusbar->showMessage(tr("Vez do Jogador %2").arg(player));
}
