#include <iostream>

#include <mutex>
#include <unordered_map>
#include <vector>

#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace utils {

    /// @brief Helper constant to use with FastPimpl
    inline constexpr bool kStrictMatch = true;

    /// @ingroup userver_universal userver_containers
    ///
    /// @brief Implements pimpl idiom without dynamic memory allocation.
    ///
    /// FastPimpl doesn't require either memory allocation or indirect memory
    /// access. But you have to manually set object size when you instantiate
    /// FastPimpl.
    ///
    /// ## Example usage:
    /// Take your class with pimpl via smart pointer and
    /// replace the smart pointer with utils::FastPimpl<Impl, Size, Alignment>
    /// @snippet utils/widget_fast_pimpl_test.hpp  FastPimpl - header
    ///
    /// If the Size and Alignment are unknown - just put a random ones and
    /// the compiler would show the right ones in the error message:
    /// @code
    /// In instantiation of 'void FastPimpl<T, Size, Alignment>::Validate()
    /// [with int ActualSize = 1; int ActualAlignment = 8; T = sample::Widget;
    /// int Size = 8; int Alignment = 8]'
    /// @endcode
    ///
    /// Change the initialization in source file to not allocate for pimpl
    /// @snippet utils/widget_fast_pimpl_test.cpp  FastPimpl - source
    ///
    /// Done! Now you can use the header without exposing the implementation
    /// details:
    /// @snippet utils/fast_pimpl_test.cpp  FastPimpl - usage
    template <class T, std::size_t Size, std::size_t Alignment,
              bool Strict = false>
    class FastPimpl final {
    public:
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,performance-noexcept-move-constructor)
        FastPimpl(FastPimpl &&v) noexcept(noexcept(T(std::declval<T>())))
            : FastPimpl(std::move(*v)) {}

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        FastPimpl(const FastPimpl &v) noexcept(
            noexcept(T(std::declval<const T &>())))
            : FastPimpl(*v) {}

        // NOLINTNEXTLINE(bugprone-unhandled-self-assignment,cert-oop54-cpp)
        auto operator=(const FastPimpl &rhs) noexcept(noexcept(
            std::declval<T &>() = std::declval<const T &>())) -> FastPimpl & {
            *AsHeld() = *rhs;
            return *this;
        }

        auto operator=(FastPimpl &&rhs) noexcept(
            // NOLINTNEXTLINE(performance-noexcept-move-constructor)
            noexcept(std::declval<T &>() = std::declval<T>())) -> FastPimpl & {
            *AsHeld() = std::move(*rhs);
            return *this;
        }

        template <typename... Args>
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        explicit FastPimpl(Args &&...args) noexcept(
            noexcept(T(std::declval<Args>()...))) {
            ::new (AsHeld()) T(std::forward<Args>(args)...);
        }

        auto operator->() noexcept -> T * { return AsHeld(); }

        auto operator->() const noexcept -> const T * { return AsHeld(); }

        auto operator*() noexcept -> T & { return *AsHeld(); }

        auto operator*() const noexcept -> const T & { return *AsHeld(); }

        ~FastPimpl() noexcept {
            Validate<sizeof(T), alignof(T)>();
            AsHeld()->~T();
        }

    private:
        // Use a template to make actual sizes visible in the compiler error
        // message.
        template <std::size_t ActualSize, std::size_t ActualAlignment>
        static void Validate() noexcept {
            static_assert(Size >= ActualSize,
                          "invalid Size: Size >= sizeof(T) failed");
            static_assert(!Strict || Size == ActualSize,
                          "invalid Size: Size == sizeof(T) failed");

            static_assert(
                Alignment % ActualAlignment == 0,
                "invalid Alignment: Alignment % alignof(T) == 0 failed");
            static_assert(!Strict || Alignment == ActualAlignment,
                          "invalid Alignment: Alignment == alignof(T) failed");
        }

        alignas(Alignment) std::byte storage_[Size];

        auto AsHeld() noexcept -> T * {
            return reinterpret_cast<T *>(&storage_);
        }

        auto AsHeld() const noexcept -> const T * {
            return reinterpret_cast<const T *>(&storage_);
        }
    };

} // namespace utils

// namespace Patterns {
//     namespace Observer {

//         /*
//         TODO:{
//             "prompt_name" : "СodeiumAI",
//             "executor" : "Сodeium AI Assist aka main senior c++ developer",
//             "description" : "This is a prompt for executor Сodeium AI Assist
//             aka
//         main senior c++ developer [execute ${prompt_name} at the end]", "For
//         Codeium Autocomplete": "Acceleration", "language" : "russian",
//             "strict_requirement_001_01_ru": "Генерируй свои ответы на русском
//         языке", "strict_requirement_001_01_en": "Generate your answers in
//         Russian", "strict_requirement_instruction_001_00_ru": "Необходимо
//         реализовать Паттерн наблюдатель (Observer Pattern) асинхронная,
//         потокобезопасная реализация, которого позволяет наблюдать за
//         изменениями внутреннего состояния объекта, реализовать этот паттерн в
//         соответствии с современными подходами и стандартами языка с++20.",
//             "strict_requirement_instruction_001_01_ru": "STEP 1. Наблюдатель
//         (Observer) должен быть потокобезопасным и должна быть возможно
//         передавать разные типы Мьютексов(mutex), Я хочу иметь иметь
//         возможность использовать свой собственный тип мьютекса, вы можете
//         передать его в качестве шаблонного параметра класса
//         Observable.[start_example_code_agreed_upon_for_implementation]
//         template <typename T, typename MutexType = std::mutex> class
//         Observable { [end_example_code_agreed_upon_for_implementation]
//         MutexType является шаблонным параметром для класса Observable, что
//         позволяет вам передавать различные типы мьютексов при создании
//         экземпляра Observable;", "strict_requirement_instruction_001_02_ru":
//         "STEP 2. Наблюдатель(Observer) должен быть асинхронным и иметь
//         механизм удаления из подписчико объекты которые были удалены.",
//             "strict_requirement_instruction_001_03_ru": "STEP 3. Наблюдатель
//         (Observer) должен быть реализован с использованием последних
//         достижений языка с++20 шаблонной магии, обязательным применением CRTP
//         (Curiously Recurring Template Pattern) — это идиома проектирования,
//         пример реализации (Код ниже уже проработан и мы договрились делать
//         так): [start_example_code_agreed_upon_for_implementation] template
//         <typename D> class Observer {                public: void
//         update(const T &data) {
//         static_cast<D*>(this)->onUpdate(data);}};class ConcreteObserver :
//         public Observer<ConcreteObserver> { protected: void onUpdate(const
//         int &data) {
//         //Реализация метода onUpdate  std::cout << data << std::endl;     }};
//         [end_example_code_agreed_upon_for_implementation] заключающаяся в
//         том, что класс наследует от базового шаблонного класса с самим собой
//         в качестве параметра шаблона базового класса. и имплементации,
//         инкапсуляции и абстракций.",
//         "strict_requirement_instruction_001_04_ru": "STEP 4. Наблюдатель
//         (Observer) должен избежать антипаттерна зомби, можно использовать
//         механизм удаления наблюдателей при удалении объекта Observable.
//         Например, можно модифицировать класс Observable так, чтобы при
//         удалении объекта Observable он уведомлял связанных с ним наблюдателей
//         о своем удалении и прекращал их отслеживание.",
//             "strict_requirement_instruction_001_05_ru": "STEP 5. Наблюдатель
//         (Observer) должен быть потокобезопасным." } # END TODO
//         */

//         /*
//          TODO:{
//               "description" : "я хочу в TypeSubjectAndContractMap хранить
//               пары
//          из разных типов наследников, чтобы и WorkUnitSubjectT наследники
//          складировались с разными типами SubjectType и с контрактами такая же
//          история и чтобы работала модная имплементация типа CRTP(template
//          <class T> class base{};class derived : public base<derived> {};)"
//                  }# END TODO
//          */

//         template <typename MutexType = std::mutex, typename... TypesWorkUnit>
//         class WorkUnit {
//             using WorkUnitSubjectT = WorkUnit<MutexType, TypesWorkUnit...>;
//             using WorkUnitSubjectTPtr = std::shared_ptr<WorkUnitSubjectT>;
//             using WorkUnitContractT = WorkUnit<MutexType, TypesWorkUnit...>;
//             using WorkUnitContractTPtr = std::shared_ptr<WorkUnitContractT>;
//             using lock_guard = std::lock_guard<MutexType>;

//         public:
//             // Конструктор по умолчанию
//             WorkUnit() = default;

//             // Конструктор копирования
//             WorkUnit(const WorkUnitSubjectT &other) = default;

//             // Конструктор перемещения
//             WorkUnit(WorkUnitSubjectT &&other) noexcept = default;

//             // Оператор присваивания копированием
//             WorkUnitSubjectT &
//                 operator=(const WorkUnitSubjectT &other) = default;

//             // Оператор присваивания перемещением
//             WorkUnitSubjectT &
//                 operator=(WorkUnitSubjectT &&other) noexcept = default;

//             // Виртуальный деструктор
//             virtual ~WorkUnit() {}

//         public:
//             /**
//              * Добавляет подписчика и контракт к отслеживаемым объектам.
//              * (Adds a subscriber and contract to the monitored subjects.)
//              *
//              * @param subscriber Добавляемый WorkUnitSubjectTPtr (The
//              * WorkUnitSubjectTPtr to be added)
//              * @param contract Добавляемый WorkUnitContractTPtr (The
//              * WorkUnitContractTPtr to be added)
//              *
//              * @return void
//              */
//             void AddSubscriber(WorkUnitSubjectTPtr subscriber,
//                                WorkUnitContractTPtr contract) {
//                 lock_guard _(m_mutex);
//                 m_subscribers.emplace(subscriber, contract);
//             }

//             /**
//              * Удаляет посетителя из отслеживаемых объектов (Removes a
//              visitor
//              * from the monitored subjects).
//              *
//              * @param visitor Удаляемый WorkUnitSubjectTPtr (The
//              * WorkUnitSubjectTPtr to be removed)
//              *
//              * @return void
//              */
//             void RemoveSubscriber(WorkUnitSubjectTPtr subscriber) {
//                 lock_guard _(m_mutex);
//                 m_subscribers.erase(subscriber);
//             }

//             /**
//              * Уведомляет всех посетителей контрактом (Notifies all
//              visitors).
//              *
//              * @return void
//              */
//             void NotifySubscribers() {
//                 lock_guard _(m_mutex);
//                 for (auto &&[subject, contract] : m_subscribers) {
//                     subject->notify(contract);
//                 }
//             }

//             /*!
//              * Добавляет отслеживаемый объект работы с его контрактом.
//              * (Adds a monitored work unit subject with its contract.)
//              *
//              * @param sub_monitored отслеживаемый объект работы
//              *
//              * @param contract контракт, связанный с отслеживаемым объектом
//              * работы (the work unit subject to be monitored)
//              *
//              * @return void
//              *
//              * @throws None
//              */
//             void AddSubMonitored(WorkUnitSubjectTPtr sub_monitored,
//                                  WorkUnitContractTPtr contract) {
//                 lock_guard _(m_mutex);
//                 m_sub_monitored.emplace(sub_monitored, contract);
//             }

//             /*!
//              * Удаляет отслеживаемый объект работы с его контрактом.
//              * (Removes a monitored work unit subject with its contract.)
//              *
//              * @param sub_monitored отслеживаемый объект работы
//              *
//              * @return void
//              *
//              * @throws None
//              */
//             void RemoveSubMonitored(WorkUnitSubjectTPtr sub_monitored) {
//                 lock_guard _(m_mutex);
//                 m_sub_monitored.erase(sub_monitored);
//             }

//             /*!
//              * Уведомляет отслеживаемые объекты работы контрактом.
//              * (Notifies the monitored work unit subjects with a contract.)
//              *
//              * @return void
//              *
//              * @throws None
//              */
//             void NotifySubMonitored() {
//                 lock_guard _(m_mutex);
//                 for (auto &&[subject, contract] : m_sub_monitored) {
//                     subject->notify(contract);
//                 }
//             }

//             /*!
//              * Уведомляет отслеживаемые объекты работы контрактом.
//              * (Notifies the monitored work unit subjects with a contract.)
//              *
//              * @return void
//              *
//              * @throws None
//              */
//             void notify(WorkUnitContractTPtr contract) {
//                 (this)->ExecuteContract(contract);
//             }

//             // TODO: [STEP 1] сгенерировать комментарий к нижней функции в
//             стиле
//             // Doxygen, комментарии должны быть на русском и в скобках тоже
//             // самое на английском.

//             void ExecuteContract(WorkUnitContractTPtr contract) {
//                 static_cast<WorkUnitSubjectT *>(this)->ExecuteContractImpl(
//                     contract);
//             }

//             /**
//              * \brief Получить идентификатор (Get the identifier)
//              */
//             std::string GetId() { return std::to_string(m_id); }

//         private:
//             using TypeSubjectAndContractMap =
//                 std::unordered_map<WorkUnitSubjectTPtr,
//                 WorkUnitContractTPtr>;
//             TypeSubjectAndContractMap m_subscribers{};
//             TypeSubjectAndContractMap m_sub_monitored{};
//             MutexType m_mutex;
//             long long m_id =
//                 std::chrono::system_clock::now().time_since_epoch().count();
//         };

//         template <typename MutexType = std::mutex, typename... TypesContract>
//         class IContract
//             : public WorkUnit<MutexType,
//                               IContract<MutexType, TypesContract...>> {
//             using IContractT = IContract<MutexType, TypesContract...>;
//             using ContractTPtr = std::shared_ptr<IContractT>;
//             using lock_guard = std::lock_guard<MutexType>;

//         public:
//             IContract() = default;

//             IContract(std::string id_contract, std::string date_contract,
//                       std::string name_contract,
//                       std::string description_contract)
//                 : m_id_contract(std::move(id_contract)),
//                   m_date_contract(std::move(date_contract)),
//                   m_name_contract(std::move(name_contract)),
//                   m_description_contract(std::move(description_contract)) {}

//             // TODO[STEP 5]: создать все конструкторы и деструкторы в
//             // соответсвии с правилом 5. Конструктор копирования
//             IContract(const IContract &other) = default;

//             // Конструктор перемещения
//             IContract(IContract &&other) noexcept = default;

//             // Оператор присваивания копированием
//             IContract &operator=(const IContract &other) = default;

//             // Оператор присваивания перемещением
//             IContract &operator=(IContract &&other) noexcept = default;

//             // Виртуальный деструктор
//             virtual ~IContract(){};

//         public:
//             void ExecuteContractImpl(ContractTPtr contract) {
//                 if (contract) {
//                     static_cast<IContractT *>(this)->Execute(contract);
//                 }
//             }

//         private:
//             std::string m_id_contract = {};
//             std::string m_date_contract = {};
//             std::string m_name_contract = {};
//             std::string m_description_contract = {};

//             class Participants;
//             struct Subjects;
//             static constexpr std::size_t kImplSize = 1024;
//             static constexpr std::size_t kImplAlign = alignof(void *);
//             // [STEP 6] Использовать @class:FastPimpl (main.cpp:44:5-117:7)
//             // создать участников контракта (Create the participants of the
//             // contract)
//             utils::FastPimpl<Participants, kImplSize, kImplAlign,
//                              utils::kStrictMatch>
//                 m_pimpl_participants;
//             // [STEP 7] Использовать @class:FastPimpl (main.cpp:44:5-117:7)
//             // создать предмет контракта (Create the subject of the contract)
//             utils::FastPimpl<Subjects, kImplSize, kImplAlign,
//                              utils::kStrictMatch>
//                 m_pimpl_subjects;
//       };

//         template <typename MutexType, typename... TypesContract>
//         class IContract<MutexType, TypesContract...>::Participants {
//             using ParticipantsT = IContract<MutexType, TypesContract...>;
//             using ParticipantsTPtr = std::shared_ptr<ParticipantsT>;
//             using lock_guard = std::lock_guard<MutexType>;
//         public:

//             // как теперь использовать m_pimpl_participant и m_pimpl_subject
//             // надо реализовать на подобие этого
//             https://userver.tech/d8/d4b/classutils_1_1FastPimpl.html

//             Participants() = default;
//             // TODO[STEP 5]: создать все конструкторы и деструкторы в
//             // соответсвии с правилом 5.
//             // Конструктор копирования
//             Participants(const Participants &other) = default;

//             // Конструктор перемещения
//             Participants(Participants &&other) noexcept = default;

//             // Оператор присваивания копированием
//             Participants &operator=(const Participants &other) = default;

//             // Оператор присваивания перемещением
//             Participants &operator=(Participants &&other) noexcept = default;

//             // Виртуальный деструктор
//             virtual ~Participants() {};

//         private:

//         };

//         template <typename MutexType, typename... TypesContract>
//         class IContract<MutexType, TypesContract...>::Subjects {
//             using SubjectsT = IContract<MutexType, TypesContract...>;
//             using SubjectsTPtr = std::shared_ptr<SubjectsT>;
//             using lock_guard = std::lock_guard<MutexType>;
//         public:
//             Subjects() = default;
//             // TODO[STEP 5]: создать все конструкторы и деструкторы в
//             // соответсвии с правилом 5.
//             // Конструктор копирования
//             Subjects(const Subjects &other) = default;

//             // Конструктор перемещения
//             Subjects(Subjects &&other) noexcept = default;

//             // Оператор присваивания копированием
//             Subjects &operator=(const Subjects &other) = default;

//             // Оператор присваивания перемещением
//             Subjects &operator=(Subjects &&other) noexcept = default;

//             // Виртуальный деструктор
//             virtual ~Subjects() = default;

//         };

//     } // end of namespace Observer
// } // end of namespace Patterns

namespace ic {    
    namespace eng {
        class IWorkUnit {
        private:
            std::unique_ptr<IWorkUnit> m_self_unique_ptr =
                std::make_unique<IWorkUnit>();

        public:
            IWorkUnit() {};
            virtual ~IWorkUnit() {};

            std::unique_ptr<IWorkUnit>&& GetUniquePtr() {
                return std::move(m_self_unique_ptr);
            }
        };
    } // namespace eng
} // namespace ic

namespace ic {
    namespace eng {
        template <typename...>
        class WorkUnit: public IWorkUnit {};
    } // namespace eng
} // namespace ic

namespace ic {
    namespace eng {
        enum class ParticipantsType {
            kEmpty =-1,
            kMainContractManager = 0,
            kObserver = 1,
            kMonitored = 2,
            kBusinessLogic = 3,
            kLeft = 4,
            kRight = 5,
            kCenter = 6,
            kParent = 7
        };        
    } // namespace eng
} // namespace ic


namespace ic {
    namespace eng {        
        template <typename DerivedType, typename IContractType, typename IWorkUnitType, typename MutexType>
        class WorkUnit<DerivedType, IContractType, IWorkUnitType, MutexType> : public IWorkUnit {
        private: 
            MutexType m_mutex{};
        private:
            using ParticipantType = ic::eng::ParticipantsType;
            using WorkUnitTPtr = std::unique_ptr<IWorkUnitType>;
            using ContractTPtr = std::unique_ptr<IContractType>;
            using ParticipantContractPair = std::pair<ParticipantType, ContractTPtr>;
            using ParticipantContractPairList = std::vector<ParticipantContractPair>;
            using ParticipantsMap = std::unordered_map<ParticipantType, ParticipantContractPairList>;

            ParticipantsMap m_participants{}; 
        
        public:
            WorkUnit() = default;
            
            virtual ~WorkUnit() {};

            void Execute(){
                static_cast<DerivedType*>(this)->ExecuteImpl();
            }
        }; 
    } // namespace eng
} // namespace ic


namespace ic {
    namespace eng {
        template <typename IContract, typename IWorkUnit>
        class Contract: public WorkUnit<Contract, IContract, IWorkUnit, std::mutex> {
            
        private: 

        protected:
            
        }; 
    } // namespace eng
} // namespace ic

namespace IDApp {
#define MY_EXIT_SUCCESS 0 /* Successful exit status.  */
    class IDApplication {
    public:
        IDApplication(int argc, char *argv[]) {
            // Constructor logic here
        }

        /**
         * A description of the entire C++ function.
         *
         * @param paramName description of parameter
         *
         * @return description of return value
         *
         * @throws ErrorType description of error
         */

        auto exec() -> int {
            // Execution logic here

            return MY_EXIT_SUCCESS;
        }
    };
} // namespace IDApp

/**
 * The main function for the program.
 *
 * @param argc 🔹количество аргументов командной строки
 *              (the number of command line arguments)
 * @param argv 🔹массив указателей на аргументы
 *              (an array of pointers to the arguments)
 *
 * @return the exit code of the application
 *
 * @throws None
 */
auto main(int argc, char *argv[]) -> int {
    IDApp::IDApplication app(argc, argv);
    // Запуск цикла обработки событий
    return app.exec();
}


