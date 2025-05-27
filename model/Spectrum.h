#ifndef INCLUDE_MODEL_SPECTRUM_H_
#define INCLUDE_MODEL_SPECTRUM_H_

#include "config.h"

#include <stddef.h>
#include <array>
#include <stdexcept>
#include <QString>

template <class Data = float, size_t N = nucare::CHSIZE>
class Spectrum_t
{
   private:
    std::array<Data, N> m_data;
    double m_acqTime = 0;
    double m_realTime = 0;
    double m_totalCount = 0;
    int m_fillCps = 0;
    int m_detID = -1;

    void update() {
        m_totalCount = m_fillCps;
        for (const auto& val : m_data) {
            m_totalCount += val;
        }
    }

   public:
    using SPC_DATA = Data*;

    Spectrum_t() noexcept : m_acqTime(1), m_totalCount(0), m_fillCps(0) {}

    explicit Spectrum_t(const Data* spc_data) noexcept(noexcept(setData(spc_data))) : Spectrum_t() {
        setData(spc_data);
    }

    Spectrum_t(const Spectrum_t& spc) = delete;

    Spectrum_t& operator=(const Spectrum_t& spc) = delete;

    ~Spectrum_t() = default;

   public:
    size_t getSize() const noexcept { return N; }

    void setAcqTime(double acqTime) noexcept { m_acqTime = acqTime; }

    void setRealTime(double realTime) noexcept { m_realTime = realTime; }

    void setData(const Data* data_ptr) {
        if (data_ptr) {
            std::copy(data_ptr, data_ptr + N, m_data.begin());
            update();
        }
    }

    void setDetectorID(int id) noexcept { m_detID = id; }

    Data* data() noexcept { return m_data.data(); }

    const Data* dataConst() const noexcept { return m_data.data(); }

    double getTotalCount() const noexcept { return m_totalCount; }

    double getAvgCps() const {
        if (m_acqTime > 0) {
            return m_totalCount / m_acqTime;
        }
        return 0.0;
    }

    void setFillCps(int cps) noexcept { m_fillCps = cps; }

    int getFillCps() const noexcept { return m_fillCps; }

    double getAcqTime() const noexcept { return m_acqTime; }

    double getRealTime() const noexcept { return m_realTime; }

    int getDetectorID() const noexcept { return m_detID; }

    void reset() {
        std::fill(m_data.begin(), m_data.end(), 0);
        m_acqTime = 1;
        m_realTime = 0;
        m_totalCount = 0;
        m_fillCps = 0;
        m_detID = -1;
    }

    void accumulate(const Spectrum_t<Data, N>& spc) {
        for (size_t i = 0; i < N; ++i) {
            m_data[i] += spc.m_data[i];
        }
        m_acqTime += spc.m_acqTime;
        m_realTime += spc.m_realTime;
        m_fillCps += spc.m_fillCps;
        update();
    }

    Spectrum_t<Data, N> operator+(const Spectrum_t<Data, N>& spc) const {
        Spectrum_t<Data, N> result = *this;
        result.accumulate(spc);
        return result;
    }

    Data& operator[](const unsigned int i) {
        if (i >= N) {
            throw std::out_of_range("Spectrum_t::operator[]: index out of bounds");
        }
        return m_data[i];
    }

    const Data& operator[](const unsigned int i) const {
        if (i >= N) {
            throw std::out_of_range("Spectrum_t::operator[] const: index out of bounds");
        }
        return m_data[i];
    }

    const Data& at(const unsigned int i) const {
        return m_data.at(i);
    }

    QString toString() {
        QString str;
        for (size_t i = 0; i < N; ++i) {
            str += QString("%1").arg(m_data[i]);
            if (i < N - 1) {
                str += ",";
            }
        }
        return str;
    }
};

typedef Spectrum_t<float, nucare::CHSIZE> Spectrum;
typedef Spectrum_t<int, nucare::HW_CHSIZE> HwSpectrum;

#endif /* INCLUDE_MODEL_SPECTRUM_H_ */
