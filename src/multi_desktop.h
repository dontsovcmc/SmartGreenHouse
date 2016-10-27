
#ifndef MULTI_DESKTOP_H
#define MULTI_DESKTOP_H

extern "C" {
  typedef void (*func)(void);
}

class Desktop
{
public: 
	Desktop(func show_, func button1_)
		: show(show_)
		, button1(button1_)
		, next(0)
	{ }
	
	Desktop *next;
	func show;
	func button1;
private:
};

class MultiDesktop
{
public:
	
	MultiDesktop(Desktop *main)
		: current(main)
		, first(main)
	{
		
	}
	
	void add_desktop(Desktop *new_d)
	{
		if (!first)
		{
			first = new_d;
		}
		else
		{
			Desktop *d = first;
			while (d->next) d = d->next;
			
			d->next = new_d;
			new_d->next = first;
		}
	}
	
	void button_pressed(char ch)
	{
		if (ch == menu::enterCode)  //NEXT desktop
		{
			if (current->next)
				current = current->next;
		}
        else if (ch == menu::upCode) //BUTTON 1
		{
			if (current->button1)
				current->button1();
		}
		
		show();
	}
	
	void show()
	{
		if (current->show)
			current->show();
	}
	
private:
	Desktop *current;
	
	Desktop *first;
};

#endif
