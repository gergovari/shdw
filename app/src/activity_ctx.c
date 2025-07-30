#include "activity_ctx.h"
#include "activity_manager.h"

#include "lv_utils.h"

#include <errno.h>
#include <stdio.h>

int shd_act_ctx_take_snapshot(shd_act_ctx_t* ctx) {
	if (ctx->snapshot != NULL) lv_draw_buf_destroy(ctx->snapshot);

	ctx->snapshot = lv_snapshot_take(ctx->screen, LV_COLOR_FORMAT_ARGB8888);
	return ctx->snapshot == NULL ? -ENOSR : 0;
}
int shd_act_ctx_screen_create(shd_act_ctx_t* ctx) {
	lv_obj_t* screen = lv_screen_create(lv_display_or_default(ctx->display));

	if (screen == NULL) {
		return -ENOSR;
	} else {
		ctx->screen = screen;
	}
	
	return 0;
}
int shd_act_ctx_screen_destroy(shd_act_ctx_t* ctx) {
	Printf("shd_act_ctx_screen_destroy: %p\n", shd_act_man_act_ctx_display_current_find(ctx));
	if (shd_act_man_act_ctx_display_current_find(ctx) == NULL) {
		printf("%s (%p) screen delete\n", ctx->activity->id, ctx);
		lv_obj_delete(ctx->screen);
		ctx->screen = NULL;
	} else return -EBUSY;

	return 0;
}

int shd_act_ctx_resume(shd_act_ctx_t* ctx) {
	shd_act_t* activity = ctx->activity;
	
	if (ctx->state != RESUMED) {
		if (activity->on_resume != NULL) activity->on_resume(ctx);
		ctx->state = RESUMED;
	}

	return 0;
}
int shd_act_ctx_pause(shd_act_ctx_t* ctx) {
	shd_act_t* activity = ctx->activity;

	if (ctx->state != STARTED_PAUSED) {
		shd_act_ctx_take_snapshot(ctx);
		// TODO: handle if we can't take snapshot?
		// FIXME: start pruning snapshots or smt to avoid running out of memory
		
		if (activity->on_pause != NULL) activity->on_pause(ctx);
		ctx->state = STARTED_PAUSED;
	}

	return 0;
}
int shd_act_ctx_start(shd_act_ctx_t* ctx) {
	int ret = 0;
	shd_act_t* activity = ctx->activity;
	
	if (ctx->state != STARTED_PAUSED) {
		ret = shd_act_ctx_screen_create(ctx);

		if (ret == 0) {
			if (activity->on_start != NULL) activity->on_start(ctx);
			ctx->state = STARTED_PAUSED;
		}
	}

	return ret;
}
int shd_act_ctx_stop(shd_act_ctx_t* ctx) {
	int ret = 0;
	shd_act_t* activity = ctx->activity;

	if (ctx->state != CREATED_STOPPED) {
		ret = shd_act_ctx_screen_destroy(ctx);

		if (ret == 0) {
			if (activity->on_stop != NULL) activity->on_stop(ctx);
			ctx->state = CREATED_STOPPED;
		}
	}

	return ret;
}
int shd_act_ctx_create(shd_act_ctx_t* ctx) {
	shd_act_t* activity = ctx->activity;

	if (ctx->state != CREATED_STOPPED) {
		if (activity->on_create != NULL) activity->on_create(ctx);
		ctx->state = CREATED_STOPPED;
	}

	return 0;
}
int shd_act_ctx_destroy(shd_act_ctx_t* ctx) {
	shd_act_t* activity = ctx->activity;

	if (ctx->state != INITIALIZED_DESTROYED) {
		if (activity->on_destroy != NULL) activity->on_destroy(ctx);
		ctx->state = INITIALIZED_DESTROYED;
	}

	return 0;
}
int shd_act_ctx_state_transition(shd_act_ctx_t* ctx, shd_act_state_t target) {
	int ret = 0;

	switch (ctx->state) {
		case INITIALIZED_DESTROYED:
			switch (target) {
				case INITIALIZED_DESTROYED: break;
				case CREATED_STOPPED:
					ret = shd_act_ctx_create(ctx);
					break;
				case STARTED_PAUSED:
					ret = shd_act_ctx_create(ctx);
					if (ret == 0) ret = shd_act_ctx_start(ctx);
					break;
				case RESUMED:
					ret = shd_act_ctx_create(ctx);
					if (ret == 0) {
						ret = shd_act_ctx_start(ctx);
						if (ret == 0) {
							ret = shd_act_ctx_resume(ctx);
						} else {
							ret = shd_act_ctx_destroy(ctx);
						}
					}
					break;
				default:
					ret = -ENOTSUP;
					break;
			}
			break;
		case CREATED_STOPPED:
			switch (target) {
				case INITIALIZED_DESTROYED: 
					ret = shd_act_ctx_destroy(ctx);
					break;
				case CREATED_STOPPED: break;
				case STARTED_PAUSED:
					ret = shd_act_ctx_start(ctx);
					break;
				case RESUMED:
					ret = shd_act_ctx_start(ctx);
					if (ret == 0) ret = shd_act_ctx_resume(ctx);
					break;
				default:
					ret = -ENOTSUP;
					break;
			}
			break;
		case STARTED_PAUSED:
			switch (target) {
				case INITIALIZED_DESTROYED: 
					ret = shd_act_ctx_stop(ctx);
					if (ret == 0) ret = shd_act_ctx_destroy(ctx);
					break;
				case CREATED_STOPPED: 
					ret = shd_act_ctx_stop(ctx);
					break;
				case STARTED_PAUSED: break;
				case RESUMED:
					ret = shd_act_ctx_resume(ctx);
					break;
				default:
					ret = -ENOTSUP;
					break;
			}
			break;
		case RESUMED:
			switch (target) {
				case INITIALIZED_DESTROYED: 
					ret = shd_act_ctx_pause(ctx);
					if (ret == 0) {
						ret = shd_act_ctx_stop(ctx);
						if (ret == 0) ret = shd_act_ctx_destroy(ctx);
					}
					break;
				case CREATED_STOPPED: 
					ret = shd_act_ctx_pause(ctx);
					if (ret == 0) ret = shd_act_ctx_stop(ctx);
					break;
				case STARTED_PAUSED:
					ret = shd_act_ctx_pause(ctx);
					break;
				case RESUMED: break;
				default:
					ret = -ENOTSUP;
					break;
			}
			break;
		default:
			ret = -ENOTSUP;
			break;
	}

	return ret;
}
