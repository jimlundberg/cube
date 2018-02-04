/****************************************************************************
**
*****************************************************************************/


#include <unistd.h>

#include "globjwin.h"
#include "glbox.h"



void displayhelp(); // called when help button clicked


GLObjectWindow::GLObjectWindow( QWidget* parent, const char* name )
    : QWidget( parent, name )
{
  HelpPage = 0;

  // first change to the ttt directory if necessary. 
  char *argv0 = qApp->argv()[0];
  
  QString *filename = new QString(argv0);
  if(filename->contains('/')){ // this would need to be changed to \ for Windows
    int index = filename->findRev('/');
    chdir(filename->left(index));

  }


    // Create a nice frame to put around the OpenGL widget
    QFrame* f = new QFrame( this, "frame" );
    f->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    f->setLineWidth( 2 );
    f->setFixedSize(310, 680);

    // Create our OpenGL widget
    GLBox* glbox = new GLBox( f, "glbox");
    glbox->setFocus();
    glbox->setFixedSize(306, 676);
    // glbox must stay a fixed size so that the mouse mapping will work
    // if you want to change this size, then a new coordinate map must be built!

    
    //QLabel *welcome = new QLabel(this);
    welcome = new QLabel(this);
    welcome->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    welcome->setText( "Welcome to 3-D tic-tac-toe!\n" );
    //welcome->setMaximumWidth(194);

    welcome->setFixedSize(194, 150);
    welcome->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    //welcome->setAlignment( Qt::AlignVCenter );
    QFont biggerfont = QFont("Times", 18);
    welcome->setFont(biggerfont);
    welcome->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop ) );

    help = new QPushButton(this);
    help->setText("Help");
    connect(help, SIGNAL(clicked()), this, SLOT(displayhelp()));


    QPushButton *quit = new QPushButton(this);
    quit->setText( "Quit" );
    connect(quit, SIGNAL(clicked()),qApp, SLOT(quit()));

    QSlider *diff = new QSlider(this);
    diff->setMinValue(2);
    diff->setMaxValue(5);
    diff->setValue(3);
    diff->setOrientation(Qt::Vertical);
    diff->setTickmarks(QSlider::Left );
    diff->setTickInterval(1);
    diff->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed)); 
    // (we don't want the slider to change in size)
    connect(diff, SIGNAL(valueChanged(int)), glbox, SLOT(dvalueChanged(int)));

    QLabel *difflabel = new QLabel(this);
    difflabel->setText( "Set difficulty level:");

    QHBoxLayout *difflayout = new QHBoxLayout(0, 20, 20, "difflayout");
    difflayout->addWidget(difflabel);
    difflayout->addWidget(diff);   

    //#### SOUND checkbox ####
    QCheckBox *soundbox = new QCheckBox("Sound", this);
    soundbox->setChecked(false);
    connect(soundbox, SIGNAL(toggled(bool)), glbox, SLOT(setSound(bool)));
    //##### end SOUND ####

    //##### code for player choice boxes (human, computer):  #####
   
    QLabel *player1label = new QLabel(this);
    player1label->setText("Player 1: ");
    player1label->setPaletteForegroundColor( QColor( 255, 0, 0 ) ); // red
    QFont boldfont(  player1label->font() );
    boldfont.setBold( TRUE );
    player1label->setFont( boldfont ); 

    QLabel *player2label = new QLabel(this);
    player2label->setText("Player 2: "); 
    player2label->setPaletteForegroundColor( QColor( 0, 255, 0 ) ); // green
    player2label->setFont( boldfont ); 

    QComboBox *player1dropdown = new QComboBox(this, "player1dropdown" );
    player1dropdown->insertItem( "Human" );
    player1dropdown->insertItem( "Computer" );
    glbox->SetPlayer1("Human"); // sets default in glbox.cpp
    glbox->SetPlayerColor(1); // Sets default as red

    QComboBox *player2dropdown = new QComboBox(this, "player2dropdown" );
    player2dropdown->insertItem( "Human" );
    player2dropdown->insertItem( "Computer" );
    glbox->SetPlayer2("Computer"); // sets default in glbox.cpp
    glbox->SetPlayerColor(2); // Sets default as green

    connect(player1dropdown, SIGNAL(highlighted(const QString &)), glbox, SLOT(SetPlayer1(const QString &)));
    connect(player2dropdown, SIGNAL(highlighted(const QString &)), glbox, SLOT(SetPlayer2(const QString &)));

    // Set up horizontal layouts for the boxes: 
    QHBoxLayout *player1box = new QHBoxLayout(0, 5, 5, "player1box");
    player1box->addWidget(player1label);
    player1box->addWidget(player1dropdown); 

    QHBoxLayout *player2box = new QHBoxLayout(0, 5, 5, "player2box");
    player2box->addWidget(player2label);
    player2box->addWidget(player2dropdown); 

    //##### end player choice boxes #######

    QPushButton *newgame = new QPushButton(this);
    newgame->setText("Start Game");
    connect(newgame, SIGNAL(clicked()), glbox, SLOT(startNewGame()));


    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );


    QVBoxLayout *vlayout = new QVBoxLayout(20, "vlayout");

    vlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    vlayout->addWidget(welcome);
    vlayout->addWidget(help);
    vlayout->addItem(spacer);
    vlayout->addLayout(player1box);
    vlayout->addLayout(player2box);
    vlayout->addLayout(difflayout);
    vlayout->addWidget(soundbox);
    vlayout->addWidget(newgame);
    vlayout->addWidget(quit);
    

    // Put the GL widget inside the frame
    QHBoxLayout* flayout = new QHBoxLayout( f, 2, 2, "flayout");
    flayout->addWidget( glbox, 1 );

    // Top level layout, puts the sliders to the left of the frame/GL widget
    QHBoxLayout* hlayout = new QHBoxLayout( this, 20, 20, "hlayout");
    // hlayout->setMenuBar( m );
    hlayout->addLayout( vlayout );
    hlayout->addWidget( f, 1 );

    //QSize size = glbox->frameSize();
    //printf("width: %d height: %d\nframe w= %d h= %d\n", glbox->width(), glbox->height(), size.width(), size.height());


    connect(glbox, SIGNAL(changeText(const QString &)), welcome, SLOT(setText(const QString &)));


}



/*
 * function called when help button is clicked. steps through help screens.
 */
void GLObjectWindow::displayhelp(){

  if(HelpPage == 0){ // (we haven't displayed help yet so display 1st page)
    QFont smallerfont = QFont("Times", 12);
    welcome->setFont(smallerfont);
    QString q("3-D Tic Tac Toe is simple to learn, but difficult to master.\nThere are 4 four-by-four square boards in a 3d space, making a total of 64 squares. To win the game you must get four of your pieces in a row along any direction. You can get a row along the x axis, y axis, z axis, or any diagonal combination.");
    welcome->setText(q);
    help->setText("Next"); // set the help button text to "Next"

  }
  else if(HelpPage == 1){ // display 2nd help page
    QString q("Before beginning a game, you can set up parameters such as the difficulty level, and who the players are.\nYou can select Human vs Computer, Computer vs Computer, or Human vs Human modes.");
    welcome->setText(q);
  }
  else if(HelpPage == 2){ 
    QString q("Increasing the difficulty level will cause the computer to \"think\" longer and it will play better by looking ahead a greater # of moves.\nYou may find that the computer is difficult to beat even on the lowest difficulty level.");
    welcome->setText(q);
  }

  else if(HelpPage == 3){ 
    QString q("Controls:\nLeft-click on a square to place your piece. Right-clicking and dragging will rotate the board in 3d space. This allows you to visualize the board from different directions. However, you can not place a piece while the board is rotated, so it will reset its position when you left click.");
    welcome->setText(q);
  }
  else if(HelpPage == 4){ 
    QString q("Tips and Hints:\nThe most essential strategy to learn in 3-D Tic Tac Toe is how to set up forks. A fork is a position in which you are threatening to win in two directions, and therefore the other player will only be able to block one way and you will win.");
    welcome->setText(q);
  }
  else if(HelpPage == 5){ 
    QString q("However, the computer will try to block most fork attempts at higher levels, so try practicing on the easiest difficulty. Also, a fun thing to watch is to set it to computer vs computer and watch how fast the game goes. Have fun!");
    welcome->setText(q);
  }
  else { // exit help
    QFont biggerfont = QFont("Times", 18);
    welcome->setFont(biggerfont);
    QString q("Press Start Game and then begin.");
    welcome->setText(q);
    help->setText("Help"); // set the help button text to "Next"
    HelpPage = 0;
    return;
  }


  HelpPage++;

}
