namespace myrisa {

struct AntipopFilter {

    float alpha_ = 0.00001f;
    float filter_;

    void trigger() {
        alpha_ = 0.0f;
    }

    float process(float in, float sample_time) {
        if (alpha_ >= 1.0f) {
            filter_ = in;
            return in;
        }

        alpha_ += sample_time * 1500; //  (args.sampleRate / 32);

        filter_ += alpha_ * (in - filter_);

        return filter_;
    }
};

} // namespace myrisa
