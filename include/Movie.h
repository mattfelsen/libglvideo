#pragma once

#include <string>
#include <map>
#include <thread>
#include "Frame.h"
#include "TrackDescription.h"
#include "GLContext.h"
#include "concurrency.h"

class AP4_File;

namespace glvideo {


/// Type alias to use for any representation of time.
typedef double seconds;

/// \class Movie
/// \brief Plays a movie from a file.
class Movie {
public:
    /// Options for a Movie
    class Options {
    public:
        Options() {}

        /// Set the number of \a frames to be buffered by the movie
        Options &bufferSize( size_t frames )
        {
            m_bufferSize = frames;
            return *this;
        }

        size_t bufferSize() const { return m_bufferSize; }

        size_t &bufferSize() { return m_bufferSize; }

    private:
        size_t m_bufferSize = 20;
    };


    typedef std::shared_ptr<Movie> ref;

    /// Returns a ref to a movie constructed from a source \a filename.
    static ref
    create( const GLContext::ref &texContext, const std::string &filename, const Options &options = Options())
    {
        return ref( new Movie( texContext, filename, options ) );
    }

    /// Constructs a movie from a \a texContext, and a source \a filename.
    ///
    /// @param  texContext  a shared GL context that will be used to create textures on a separate thread.
    /// @param  filename    the filename to read from.
    ///
    /// \throws Error if the file cannot be read
    Movie( const GLContext::ref &texContext, const std::string &filename, const Options &options = Options());

    ~Movie();

    /// Returns a string representation of the container format (eg. "qt 512").
    std::string getFormat() const;

    /// Returns the number of tracks found in the container.
    size_t getNumTracks() const;

    /// Returns the TrackDescription for the track at \a index. Indexes start at 0, and are internally mapped to track ids.
    TrackDescription getTrackDescription( size_t index ) const;

    /// Returns the length of the movie in seconds.
    seconds getDuration() const;

    /// Starts playing the movie.
    void play();

    /// Is the movie playing?
    bool isPlaying() const { return m_isPlaying; }

    /// Stops playing the movie, terminating the read thread.
    void stop();

    /// Pauses playing the movie, but leaves the read thread running. Same as setting the playback rate to 0.
    void pause();

    /// Returns the current Frame.
    Frame::ref getCurrentFrame();

private:
    /// A pointer to the AP4 file object.
    AP4_File *m_file = NULL;
    /// A mapping of indexes to track ids. Indexes start at 0.
    std::map<size_t, uint32_t> m_trackIndexMap;
    /// User-supplied options for the movie.
    Options m_options;
    /// The current frame.
    Frame::ref m_currentFrame = nullptr;
    /// A shared GL context that will be used for threaded texture creation.
    GLContext::ref m_texContext = nullptr;

    /// Extracts and decodes the sample with index \a i_sample from track with index \a i_track and returns a Frame.
    Frame::ref getFrame( size_t i_track, size_t i_sample ) const;

    /// Reads frames into the frame buffer on a thread.
    void read( GLContext::ref context );

    /// Queues frames from the frame buffer into m_currentFrame.
    void queueFrames();

    concurrent_buffer<Frame::ref> m_frameBuffer;
    concurrent_queue<Frame::ref> m_frameQueue;
    std::thread m_readThread;
    std::thread m_queueThread;
    std::atomic_bool m_isPlaying{false};

    /// Non-threadsafe member variables (for use only within their threads)

    size_t mt_sample = 0;

};
}