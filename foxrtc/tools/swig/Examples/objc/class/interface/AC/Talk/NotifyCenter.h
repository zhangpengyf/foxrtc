#ifndef NotifyCenter_H__
#define NotifyCenter_H__
#include "AC/Talk/UserAgentTypes.h"
#include "AC/Talk/Event/EventArgs.h"
namespace AC
{
	namespace Talk
	{
		class AbstractObserver
		{
		public:
			AbstractObserver(){}
			AbstractObserver(const AbstractObserver& observer){}

			AbstractObserver& operator = (const AbstractObserver& observer)
			{
				return *this;
			}
		public:
			virtual void Notify(AC::Talk::Event::ACEventArgs* args) = 0;
			virtual int Priority() = 0;
			virtual int Id() = 0;
			virtual	void* Receiver() = 0;
			virtual	std::string EventName()const = 0;
			virtual bool Equals(const AbstractObserver& observer) = 0;
			virtual AbstractObserver* Clone() = 0;
		};
		template<typename TReceiver,typename TArgs>
		class TypeObserver:public AbstractObserver
		{
		public:
			typedef void(TReceiver::*OnEventCallback)(TArgs*);

			TypeObserver(TReceiver* receiver,OnEventCallback callback,int id = 0,int priority = 0)
			{
				_receiver = receiver;
				_callback = callback;
				_priority = priority;
				_id = id;
			}

			~TypeObserver(){}

			TypeObserver& operator = (const TypeObserver& observer)
			{
				if (&observer!=this)
				{
					_callback = observer._callback;
					_id = observer._id;
					_receiver = observer._receiver;
					_priority = observer._priority;
				}
				return *this;
			}

			virtual void Notify(AC::Talk::Event::ACEventArgs* args)
			{
				if (_receiver)
				{
				    TArgs* castArgs = dynamic_cast<TArgs*>(args);
					if(castArgs)
					{
						(_receiver->*_callback)(castArgs);
					}
				}
			}

			virtual int Priority()
			{
				return _priority;
			}

			virtual int Id()
			{
				return _id;
			}

			virtual std::string EventName() const
			{
				return TArgs::TypeName();
			}

			virtual void* Receiver()
			{
				return _receiver;
			}
			virtual bool Equals(const AbstractObserver& observer)
			{
				const TypeObserver* pObs = dynamic_cast<const TypeObserver*>(&observer);
				return pObs && pObs->_receiver == _receiver && pObs->_callback == _callback&&EventName()== pObs->EventName();
			}

			virtual AbstractObserver* Clone()
			{
				return new TypeObserver(*this);
			}

		private:
			TReceiver* _receiver;
			OnEventCallback _callback;
			int _priority;
			int _id;
		};
		template<typename TReceiver>
		class NameObserver:public AbstractObserver
		{
		public: 
			typedef void (TReceiver::*OnEventCallback)(AC::Talk::Event::ACEventArgs*);
			NameObserver(TReceiver* receiver,std::string eventName,OnEventCallback onEvent,int id = 0,int priority = 0)
			{
				_receiver = receiver;
				_callback = onEvent;
				_eventName = eventName;
				_id = id;
				_priority = priority;
			}
			~NameObserver(){}

			NameObserver& operator = (const NameObserver& observer)
			{
				if (&observer!=this)
				{
					_callback = observer._callback;
					_eventName = observer._eventName;
					_id = observer._id;
					_priority = observer._priority;
					_receiver = observer._receiver;
				}
				return *this;
			}
			virtual void Notify(AC::Talk::Event::ACEventArgs* args)
			{
				if (_receiver)
				{
					(_receiver->*_callback)(args);
				}
			}

			virtual int Priority()
			{
				return _priority;
			}

			virtual int Id()
			{
				return _id;
			}

			virtual std::string EventName() const
			{
				return _eventName;
			}

			virtual void* Receiver()
			{
				return _receiver;
			}
			virtual bool Equals(const AbstractObserver& observer)
			{
				const NameObserver* pObs = dynamic_cast<const NameObserver*>(&observer);
				return pObs && pObs->_receiver == _receiver && pObs->_callback == _callback&&EventName()==pObs->EventName();
			}
		private:
			TReceiver* _receiver;
			OnEventCallback _callback;
			int _priority;
			int _id;
			std::string _eventName;
		};
		
		template<typename TReceiver,typename TArgs>
	    inline TypeObserver<TReceiver,TArgs> Observer(TReceiver* receiver,void(TReceiver::*notifyMethod)(TArgs*))
		{
			return TypeObserver<TReceiver,TArgs>(receiver,notifyMethod);
		}
		template<typename TReceiver,typename TArgs>
		inline TypeObserver<TReceiver,TArgs> Observer(TReceiver* receiver,void(TReceiver::*notifyMethod)(TArgs*),int id)
		{
			return TypeObserver<TReceiver,TArgs>(receiver,notifyMethod,id);
		}
		template<typename TReceiver,typename TArgs>
		inline TypeObserver<TReceiver,TArgs> Observer(TReceiver* receiver,void(TReceiver::*notifyMethod)(TArgs*),int id,int priority)
		{
			return TypeObserver<TReceiver,TArgs>(receiver,notifyMethod,id,priority);
		}

		template<typename TReceiver>
		inline NameObserver<TReceiver> Observer(TReceiver* receiver,std::string eventName,void(TReceiver::*notifyMethod)(AC::Talk::Event::ACEventArgs*))
		{
			return NameObserver<TReceiver>(receiver,eventName,notifyMethod);
		}
		template<typename TReceiver>
		inline NameObserver<TReceiver> Observer(TReceiver* receiver,std::string eventName,void(TReceiver::*notifyMethod)(AC::Talk::Event::ACEventArgs*),int id)
		{
			return NameObserver<TReceiver>(receiver,eventName,notifyMethod,id);
		}

		template<typename TReceiver>
		inline NameObserver<TReceiver> Observer(TReceiver* receiver,std::string eventName,void(TReceiver::*notifyMethod)(AC::Talk::Event::ACEventArgs*),int id,int priority)
		{
			return NameObserver<TReceiver>(receiver,eventName,notifyMethod,id,priority);
		}


		class NotifyCenter
		{
		public:
			static NotifyCenter* Instance();
		public:
			virtual void Register(AbstractObserver& observer) = 0;
			virtual void UnRegister(void* receiver) = 0;
		    virtual	void UnRegister(void* receiver, std::string eventName) = 0;
			virtual void UnRegister(AbstractObserver& observer) = 0;
			virtual void Dispatch(AC::Talk::Event::ACEventArgs& eventArgs) = 0;
		};

		class LocalNotifyCenter
		{
		private:
			std::vector<AbstractObserver*> _observers;
		public:
			void Register(AbstractObserver& observer)
			{
				_observers.push_back(observer.Clone());
				NotifyCenter::Instance()->Register(observer);
			}
			~LocalNotifyCenter()
			{
				for (unsigned int i =0;i<_observers.size();i++)
				{
				   NotifyCenter::Instance()->UnRegister(*_observers[i]);
				   delete _observers[i];
				}
				_observers.clear();
			}
		};

		template<typename TReceiver>
		class TLocalNotifyCenter
		{
		public:
			TLocalNotifyCenter(TReceiver* receiver)
			{
				_receiver = receiver;
			}
			template<typename TEventArgs>
			void Register(void(TReceiver::*CallBack)(TEventArgs*),int id = 0,int priority = 0)
			{
				TypeObserver<TReceiver,TEventArgs> ob = Observer<TReceiver,TEventArgs>(_receiver,CallBack,id,priority);
				_observers.push_back(ob.Clone());
				NotifyCenter::Instance()->Register(ob);
			}
			template<typename TEventArgs>
			void UnRegister(void(TReceiver::*CallBack)(TEventArgs*),int id = 0,int priority = 0)
			{
				TypeObserver<TReceiver,TEventArgs> ob = Observer<TReceiver,TEventArgs>(_receiver,CallBack,id,priority);
				NotifyCenter::Instance()->UnRegister(ob);
				//no need remove from _observers;
			}
			void UnRegisterAll()
			{
				for (unsigned int i =0;i<_observers.size();i++)
				{
					NotifyCenter::Instance()->UnRegister(*_observers[i]);
					delete _observers[i];
				}
				_observers.clear();
			}
			~TLocalNotifyCenter()
			{
				UnRegisterAll();
			}
		private:
			TReceiver* _receiver;
			std::vector<AbstractObserver*> _observers;
		};
	}
}

#endif // NotifyCenter_H__
