// backend_cpp/include/transport_models.h
#ifndef TRANSPORT_MODELS_H
#define TRANSPORT_MODELS_H

#include <string>

// 1. АБСТРАКЦІЯ (Abstract Base Class)
// Визначає контракт для всіх типів транспорту
class ITransportModel {
public:
    virtual ~ITransportModel() = default; // Віртуальний деструктор

    // Чисто віртуальні методи (must be implemented)
    virtual double getDwellTime() const = 0;      // Час очікування на зупинці
    virtual double getSpeedFactor() const = 0;    // Коефіцієнт швидкості
    virtual std::string getIcon() const = 0;      // Іконка для логів/дебагу
};

// 2. УСПАДКУВАННЯ (Bus inheriting from ITransportModel)
class BusModel : public ITransportModel {
public:
    // 3. ПЕРЕВИЗНАЧЕННЯ (Override)
    double getDwellTime() const override {
        return 5.0; // Автобус стоїть 5 секунд (стандарт)
    }
    double getSpeedFactor() const override {
        return 1.0; // 100% швидкості
    }
    std::string getIcon() const override {
        return "🚌";
    }
};

class TramModel : public ITransportModel {
public:
    double getDwellTime() const override {
        return 8.0; // Трамвай довше висаджує пасажирів (Поліморфізм даних)
    }
    double getSpeedFactor() const override {
        return 0.8; // Трамвай їде повільніше (80% від номіналу)
    }
    std::string getIcon() const override {
        return "🚊";
    }
};

class TrolleybusModel : public ITransportModel {
public:
    double getDwellTime() const override {
        return 6.0;
    }
    double getSpeedFactor() const override {
        return 0.9; // Трохи повільніше автобуса
    }
    std::string getIcon() const override {
        return "🚎";
    }
};

#endif // TRANSPORT_MODELS_H