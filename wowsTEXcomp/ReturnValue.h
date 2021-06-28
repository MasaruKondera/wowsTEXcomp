#pragma once
public ref class ReturnValue
{
public:
	enum class Parser : int {
		SUCCESS,
		PRINT_USAGE,
		NO_USAGE,
		THIS_EXE_PATH_TOO_LONG,
		THIS_EXE_PATH_ERROR,
		UNPACKER_TARGET_TOO_LONG,
		WORKING_DIR_TOO_LONG,
		STRING_CONVERSION_ERROR,
		PREFIX_INCORRECT,
		FORMAT_EXCEPTION,
		HELP_NOT_ALONE,
		MISSING_ARGUMENT,
		GAME_EXE_NOT_FOUND,
		UNPACKER_NOT_FOUND,
		BIN_NOT_FOUND,
		BIN_FORMAT,
		BIN_OVERFLOW,
		BIN_PATH_TOO_LONG,
		BIN_NO_ACCESS,
		IDX_MISSING,
		BIN_EMPTY,
		MISSING_PKG,
		EMPTY_GAME_LOCATION,
		GAME_LOCATION_NOT_ABSOLUTE,
		SCALE_TOO_BIG,
		GAME_LOCATION_INVALID_PATH,
		ZIP_NOT_ABSOLUTE,
		ZIP_ARGUMENT_EXCEPTION,
		ZIP_PATH_TOO_LONG,
		ZIP_PATH_NOT_FOUND,
		ZIP_ACCESS_FAILURE,
		ZIP_PATH_IS_FILE,
		ZIP_PROTECTED,
		ZIP_PATH_EMPTY,
		ZIP_INVALID_PATH,
		ZIP_INVALID_NAME,
		UNPACK_NOT_ABSOLUTE,
		UNPACK_ARGUMENT_EXCEPTION,
		UNPACK_PATH_TOO_LONG,
		UNPACK_PATH_NOT_FOUND,
		UNPACK_ACCESS_FAILURE,
		UNPACK_PATH_IS_FILE,
		UNPACK_PROTECTED,
		UNPACK_PATH_EMPTY,
		UNPACK_INVALID_PATH,
		WORK_DIR_NOT_ABSOLUTE,
		WORK_DIR_ARGUMENT_EXCEPTION,
		WORK_DIR_PATH_TOO_LONG,
		WORK_DIR_PATH_NOT_FOUND,
		WORK_DIR_ACCESS_FAILURE,
		WORK_DIR_PATH_IS_FILE,
		WORK_DIR_PROTECTED,
		WORK_DIR_PATH_EMPTY,
		WORK_DIR_INVALID_PATH
	};

	enum class Console : int {
		CONTINUE,
		ABORT
	};

	enum class Unpacker : int {
		SUCCESS,
		WAITING_TO_START,
		STARTED,
		FAILURE
	};

	enum class Texture : int {
		SUCCESS,
		LOAD_FAIL,
		DECOMPRESS_FAIL,
		RESIZE_FAIL,
		MIPMAP_FAIL,
		COMPRESS_FAIL,
		SAVE_FAIL,
		ABORT,
		THREAD_STATE,
		THREAD_MEMORY,
		THREAD_INTERRUPTED
	};

	enum class Zipper : int {
		SUCCESS,
		THREAD_STATE,
		THREAD_MEMORY,
		PATH_TOO_LONG,
		IO_EXCEPTION,
		NO_ACCESS,
		NO_RESULT
	};

	enum class Delete : int {
		SUCCESS,
		THREAD_STATE,
		THREAD_MEMORY,
		IO_EXCEPTION,
		NO_ACCESS,
		NO_RESULT
	};
};
