/*! Tree-climbing call.
*/
void GeoOp::build_handles(ViewerContext* ctx) {
    if (ctx->transform_mode()==VIEWER_2D) return;

    int saved_connected = ctx->connected();
    if (ctx->viewer_mode()!=VIEWER_2D && !node_disabled() && saved_connected>=SHOW_OBJECT) {
        validate(false);
        add_draw_geometry(ctx);
        // make inputs not draw the objects:
        ctx->connected(CONNECTED);
    }
    #if 0
    build_input_handles(ctx);
    #else
    for (int i=0; i < inputs(); i++) {
        Iop* iop = dynamic_cast<Iop*>(Op::input(i));
        if (!iop) {
            add_input_handle(i, ctx);
        } else {
            // Switch viewer to 2D mode:
            int saved_mode = ctx->transform_mode();
            ctx->transform_mode(VIEWER_2D);
            int saved_connected = ctx->connected();
            ctx->connected(SHOW_PUSHED_OBJECT);

            // Scale the modelmatrix to fit the input_iop's format:
            Matrix4 saved_modelmatrix(ctx->modelmatrix); // So we can restore it
            ctx->modelmatrix.scale(0.5, 0.5, 1);
            CameraOp::from_format(ctx->modelmatrix, &(iop->info().format()));

            // Set perspective value to 1 which stops renderers feeding 3D objects
            // from displaying their 3D scene:
            ctx->modelmatrix.a32 = 1.0f;

            add_input_handle(i, ctx);

            // Switch viewer back to 3D mode:
            ctx->transform_mode(saved_mode);
            ctx->modelmatrix = saved_modelmatrix;
            ctx->connected(saved_connected);
        }
    }
    #endif
    build_knob_handles(ctx);
    ctx->connected(saved_connected);
}
